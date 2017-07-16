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
 * @brief               Vulkan command buffer management.
 *
 * TODO:
 *  - Any benefit to keeping around command buffers for reuse by resetting them
 *    rather than creating/freeing? Should at least be a finite number of them.
 *  - Same goes for fences.
 */

#include "command_buffer.h"
#include "manager.h"

/** Create a command pool.
 * @param manager       Manager that owns this command pool. */
VulkanCommandPool::VulkanCommandPool(VulkanGPUManager *manager) :
    VulkanObject (manager)
{
    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                       VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = manager->device()->queueFamily();

    checkVk(vkCreateCommandPool(manager->device()->handle(), &createInfo, nullptr, &m_transientPool));
}

/** Destroy the command pool. */
VulkanCommandPool::~VulkanCommandPool() {
    vkDestroyCommandPool(manager()->device()->handle(), m_transientPool, nullptr);
}

/**
 * Allocate a transient command buffer.
 *
 * Allocates a transient command buffer for use within the current frame only.
 * It will automatically be freed as soon as possible (immediately at the start
 * of the next frame if the buffer was not submitted, otherwise as soon as the
 * submission completes).
 *
 * @param level         Level for the command buffer.
 *
 * @return              Allocated command buffer.
 */
VulkanCommandBuffer *VulkanCommandPool::allocateTransient(VkCommandBufferLevel level) {
    auto &currentFrame = manager()->currentFrame();

    VulkanCommandBuffer *buffer = new VulkanCommandBuffer(this, level, true);
    currentFrame.cmdBuffers.push_back(buffer);
    return buffer;
}

/** Clean up a previous frame's data.
 * @param frame         Frame to clean up.
 * @param completed     Whether the frame has been completed. */
void VulkanCommandPool::cleanupFrame(VulkanFrame &frame, bool completed) {
    for (auto i = frame.cmdBuffers.begin(); i != frame.cmdBuffers.end(); ) {
        VulkanCommandBuffer *buffer = *i;

        /* Free unsubmitted buffers or all buffers if completed. */
        if (buffer->m_state != VulkanCommandBuffer::State::kSubmitted || completed) {
            if (buffer->m_state == VulkanCommandBuffer::State::kSubmitted)
                buffer->m_state = VulkanCommandBuffer::State::kAllocated;

            delete buffer;
            frame.cmdBuffers.erase(i++);
        } else {
            ++i;
        }
    }
}

/** Create a new command buffer.
 * @param pool          Pool the command buffer is being allocated from.
 * @param level         Command buffer level.
 * @param transient     Whether the buffer is transient. */
VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandPool *pool, VkCommandBufferLevel level, bool transient) :
    VulkanHandle (pool->manager()),
    m_pool       (pool),
    m_transient  (transient),
    m_state      (State::kAllocated)
{
    // TODO: Always transient for now. Fix in destructor as well.
    check(transient);

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = m_pool->m_transientPool;
    allocateInfo.level = level;
    allocateInfo.commandBufferCount = 1;

    checkVk(vkAllocateCommandBuffers(manager()->device()->handle(), &allocateInfo, &m_handle));
}

/** Destroy the command buffer. */
VulkanCommandBuffer::~VulkanCommandBuffer() {
    check(m_state != State::kSubmitted);
    vkFreeCommandBuffers(manager()->device()->handle(), m_pool->m_transientPool, 1, &m_handle);
}

/** Begin recording a command buffer.
 * @param usage         Usage flags.
 * @param inheritance   For a secondary command buffer, inheritance information. */
void VulkanCommandBuffer::begin(VkCommandBufferUsageFlags usage,
                                const VkCommandBufferInheritanceInfo *inheritance)
{
    check(m_state == State::kAllocated);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usage;
    beginInfo.pInheritanceInfo = inheritance;

    checkVk(vkBeginCommandBuffer(m_handle, &beginInfo));
    m_state = State::kRecording;
}

/** Finish recording a command buffer. */
void VulkanCommandBuffer::end() {
    check(m_state == State::kRecording);

    checkVk(vkEndCommandBuffer(m_handle));
    m_state = State::kRecorded;
}

/**
 * Add an object reference.
 *
 * This adds a reference to the specified object which ensures that it will not
 * be freed until the command buffer is destroyed (either has completed
 * execution or is discarded).
 *
 * @param object        Object to reference.
 */
void VulkanCommandBuffer::addReference(Refcounted *object) {
    m_references.emplace_back(object);
}

/**
 * Add a buffer reference.
 *
 * Special case of addReference() to add a reference for a buffer, as buffers
 * have special behaviour (their current allocations must also be referenced).
 *
 * @param buffer        Buffer to reference.
 */
void VulkanCommandBuffer::addReference(GPUBuffer *buffer) {
    /* Buffer memory allocation lifetime is not tied directly to the buffer
     * object lifetime due to invalidation, so we must reference both the buffer
     * and its current allocation. */
    addReference(static_cast<Refcounted *>(buffer));
    addReference(static_cast<VulkanBuffer *>(buffer)->allocation());
}
