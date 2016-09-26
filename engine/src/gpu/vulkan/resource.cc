/*
 * Copyright (C) 2016 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               Vulkan resource set implementation.
 */

#include "manager.h"
#include "resource.h"

/** Number of resource sets/descriptors to allocate. */
static constexpr uint32_t kMaxDescriptorSets = 4096;
static constexpr uint32_t kMaxUniformBufferDescriptors = 2048;
static constexpr uint32_t kMaxImageSamplerDescriptors = 2048;

/** Initialise the resource set layout.
 * @param manager       Manager that owns the resource set layout.
 * @param desc          Descriptor for the layout. */
VulkanResourceSetLayout::VulkanResourceSetLayout(VulkanGPUManager *manager, GPUResourceSetLayoutDesc &&desc) :
    GPUResourceSetLayout(std::move(desc)),
    VulkanHandle(manager)
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(m_desc.slots.size());

    for (size_t i = 0; i < m_desc.slots.size(); i++) {
        auto &slot = m_desc.slots[i];

        if (slot.type == GPUResourceType::kNone)
            continue;

        bindings.emplace_back();
        auto &binding = bindings.back();
        binding.binding = i;

        switch (slot.type) {
            case GPUResourceType::kUniformBuffer:
                binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case GPUResourceType::kTexture:
                binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
            default:
                unreachable();
        }

        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_ALL;
    }

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = bindings.size();
    createInfo.pBindings = &bindings[0];

    checkVk(vkCreateDescriptorSetLayout(
        manager->device()->handle(),
        &createInfo,
        nullptr,
        &m_handle));
}

/** Destroy the resource set layout. */
VulkanResourceSetLayout::~VulkanResourceSetLayout() {
    vkDestroyDescriptorSetLayout(manager()->device()->handle(), m_handle, nullptr);
}

/** Create the descriptor pool.
 * @param manager       Manager that owns the object. */
VulkanDescriptorPool::VulkanDescriptorPool(VulkanGPUManager *manager) :
    VulkanHandle(manager)
{
    // TODO: This probably needs reworking in future, we can run out of
    // descriptors. Also, for multithreading we'll want per-thread pools.

    std::vector<VkDescriptorPoolSize> poolSizes(2);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = kMaxUniformBufferDescriptors;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = kMaxImageSamplerDescriptors;

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets = kMaxDescriptorSets;
    createInfo.poolSizeCount = poolSizes.size();
    createInfo.pPoolSizes = &poolSizes[0];

    checkVk(vkCreateDescriptorPool(manager->device()->handle(), &createInfo, nullptr, &m_handle));
}

/** Destroy the descriptor pool. */
VulkanDescriptorPool::~VulkanDescriptorPool() {
    vkDestroyDescriptorPool(manager()->device()->handle(), m_handle, nullptr);
}

/** Initialise the resource set.
 * @param manager       Manager that owns the resource set.
 * @param layout        Layout for the resource set. */
VulkanResourceSet::VulkanResourceSet(VulkanGPUManager *manager, GPUResourceSetLayout *layout) :
    GPUResourceSet(layout),
    VulkanObject(manager),
    m_dirtySlots(m_slots.size(), true),
    m_bufferBindings(m_slots.size(), 0)
{}

/** Destroy the resource set. */
VulkanResourceSet::~VulkanResourceSet() {
    /* If we have a current resource set object it will be released by
     * ReferencePtr automatically, and will be freed once it is no longer
     * referenced by any command lists. */
}

/** Initialise the descriptor set.
 * @param manager       Manager that owns the object.
 * @param layout        Layout for the set. */
VulkanResourceSet::DescriptorSet::DescriptorSet(
    VulkanGPUManager *manager,
    const VulkanResourceSetLayout *layout)
    :
    VulkanHandle(manager)
{
    // TODO: Need to handle failure. Pools can be exhausted, or can become
    // fragmented causing an allocation failure.
    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = manager->descriptorPool()->handle();
    allocateInfo.descriptorSetCount = 1;
    VkDescriptorSetLayout layoutHandle = layout->handle();
    allocateInfo.pSetLayouts = &layoutHandle;
    checkVk(vkAllocateDescriptorSets(manager->device()->handle(), &allocateInfo, &m_handle));
}

/** Destroy the descriptor set. */
VulkanResourceSet::DescriptorSet::~DescriptorSet() {
    vkFreeDescriptorSets(
        manager()->device()->handle(),
        manager()->descriptorPool()->handle(),
        1, &m_handle);
}

/** Update a slot's binding.
 * @param index         Index of the slot that was changed. */
void VulkanResourceSet::updateSlot(size_t index) {
    m_dirtySlots[index] = true;
}

/**
 * Perform pending updates.
 *
 * Apply pending updates before a draw using the resource set, and return the
 * Vulkan descriptor set handle that should be used for the draw. Since updating
 * may result in a new descriptor set being created if the previous one was in
 * use, this may return a different handle, in which case it must be rebound on
 * the command buffer.
 *
 * The specified command buffer will have references to the underlying
 * descriptor set object added, along with all resources bound in the resource
 * set. The resource set will not be bound on the command buffer however, this
 * is done by the caller.
 *
 * @param cmdBuf        The command buffer that will use the resource set.
 *
 * @return              Descriptor set handle to use.
 */
VkDescriptorSet VulkanResourceSet::prepareForDraw(VulkanCommandBuffer *cmdBuf) {
    /* Determine what we need to do, if anything. */
    bool needUpdate = false;
    bool needNew = false;
    if (m_current) {
        for (size_t i = 0; i < m_slots.size(); i++) {
            const Slot &slot = m_slots[i];

            /* If this resource slot is a buffer, we need to check if we have
             * reallocated the buffer since we bound it, in which case we do
             * need to update the descriptor. */
            switch (slot.desc.type) {
                case GPUResourceType::kUniformBuffer:
                    if (!m_dirtySlots[i] && slot.object) {
                        auto buffer = static_cast<VulkanBuffer *>(slot.object.get());
                        m_dirtySlots[i] = m_bufferBindings[i] != buffer->generation();
                    }

                    break;
                default:
                    break;
            }

            if (m_dirtySlots[i])
                needUpdate = true;
        }

        if (needUpdate) {
            /* Need a new descriptor set if the current one is in use, indicated
             * by a reference count greater than 1 (the 1 comes from our current
             * pointer, any more means a command buffer is referencing it). */
            needNew = m_current->refcount() > 1;
        }
    } else {
        /* Don't currently have a descriptor set. */
        needNew = true;
        needUpdate = true;
    }

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    std::vector<VkCopyDescriptorSet> descriptorCopies;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkDescriptorImageInfo> imageInfos;

    ReferencePtr<DescriptorSet> prev;

    if (needNew) {
        descriptorCopies.reserve(m_slots.size());

        /* Save the previous set pointer as we're going to do copies from the
         * unchanged descriptors, so it must stay alive until after the update. */
        prev = std::move(m_current);

        m_current = new DescriptorSet(manager(), static_cast<VulkanResourceSetLayout *>(m_layout.get()));

        /* Copy unchanged descriptors. Note that if this is the first descriptor
         * set we have created all dirty flags will be set to true by our
         * constructor. */
        for (size_t i = 0; i < m_slots.size(); i++) {
            if (!m_dirtySlots[i]) {
                const Slot &slot = m_slots[i];

                if (!slot.object)
                    continue;

                check(prev);

                descriptorCopies.emplace_back();
                auto &copy = descriptorCopies.back();
                copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
                copy.srcSet = prev->handle();
                copy.srcBinding = i;
                copy.srcArrayElement = 0;
                copy.dstSet = m_current->handle();
                copy.dstBinding = i;
                copy.dstArrayElement = 0;
                copy.descriptorCount = 1;
            }
        }
    }

    if (needUpdate) {
        descriptorWrites.reserve(m_slots.size());
        bufferInfos.reserve(m_slots.size());
        imageInfos.reserve(m_slots.size());

        for (size_t i = 0; i < m_slots.size(); i++) {
            if (m_dirtySlots[i]) {
                const Slot &slot = m_slots[i];

                if (!slot.object)
                    continue;

                descriptorWrites.emplace_back();
                auto &write = descriptorWrites.back();
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = m_current->handle();
                write.dstBinding = i;
                write.dstArrayElement = 0;
                write.descriptorCount = 1;

                switch (slot.desc.type) {
                    case GPUResourceType::kUniformBuffer:
                    {
                        auto buffer = static_cast<VulkanBuffer *>(slot.object.get());
                        VulkanMemoryManager::BufferMemory *allocation = buffer->allocation();

                        bufferInfos.emplace_back();
                        auto &bufferInfo = bufferInfos.back();
                        bufferInfo.buffer = allocation->buffer();
                        bufferInfo.offset = allocation->offset();
                        bufferInfo.range = buffer->size();

                        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        write.pBufferInfo = &bufferInfo;
                        break;
                    }

                    case GPUResourceType::kTexture:
                    {
                        auto texture = static_cast<VulkanTexture *>(slot.object.get());
                        auto sampler = static_cast<VulkanSamplerState *>(slot.sampler.get());

                        imageInfos.emplace_back();
                        auto &imageInfo = imageInfos.back();
                        imageInfo.sampler = sampler->handle();
                        imageInfo.imageView = texture->resourceView();
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write.pImageInfo = &imageInfo;
                        break;
                    }

                    default:
                        unreachable();
                }

                m_dirtySlots[i] = false;
            }
        }
    }

    /* Apply the updates. */
    vkUpdateDescriptorSets(
        manager()->device()->handle(),
        descriptorWrites.size(), &descriptorWrites[0],
        descriptorCopies.size(), &descriptorCopies[0]);

    /* The command buffer will be using this resource set, reference it. */
    cmdBuf->addReference(m_current);

    /* Furthermore, it may be using all resources bound in the set. */
    for (const Slot &slot : m_slots) {
        if (!slot.object)
            continue;

        switch (slot.desc.type) {
            case GPUResourceType::kUniformBuffer:
                cmdBuf->addReference(static_cast<VulkanBuffer *>(slot.object.get()));
                break;
            case GPUResourceType::kTexture:
                cmdBuf->addReference(static_cast<VulkanTexture *>(slot.object.get()));
                cmdBuf->addReference(slot.sampler);
                break;
            default:
                unreachable();
        }
    }

    return m_current->handle();
}

/** Create a resource set layout.
 * @param desc          Descriptor for the layout.
 * @return              Pointer to created resource set layout. */
GPUResourceSetLayoutPtr VulkanGPUManager::createResourceSetLayout(GPUResourceSetLayoutDesc &&desc) {
    return new VulkanResourceSetLayout(this, std::move(desc));
}

/** Create a resource set.
 * @param layout        Layout for the resource set.
 * @return              Pointer to created resource set. */
GPUResourceSetPtr VulkanGPUManager::createResourceSet(GPUResourceSetLayout *layout) {
    return new VulkanResourceSet(this, layout);
}
