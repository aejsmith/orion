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

#include "device.h"
#include "resource.h"

/** Initialise the resource set layout.
 * @param desc          Descriptor for the layout. */
VulkanResourceSetLayout::VulkanResourceSetLayout(GPUResourceSetLayoutDesc &&desc) :
    GPUResourceSetLayout(std::move(desc))
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(desc.slots.size());

    for (size_t i = 0; i < desc.slots.size(); i++) {
        auto &slot = desc.slots[i];

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
                check(false);
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
        g_vulkan->device()->handle(),
        &createInfo,
        nullptr,
        &m_handle));
}

/** Destroy the resource set layout. */
VulkanResourceSetLayout::~VulkanResourceSetLayout() {
    vkDestroyDescriptorSetLayout(g_vulkan->device()->handle(), m_handle, nullptr);
}

/** Create a resource set layout.
 * @param desc          Descriptor for the layout.
 * @return              Pointer to created resource set layout. */
GPUResourceSetLayoutPtr VulkanGPUManager::createResourceSetLayout(GPUResourceSetLayoutDesc &&desc) {
    return new VulkanResourceSetLayout(std::move(desc));
}
