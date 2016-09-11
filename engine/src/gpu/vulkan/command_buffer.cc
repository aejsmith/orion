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

/** Create a command pool.
 * @param manager       Manager that owns this command pool. */
VulkanCommandPool::VulkanCommandPool(VulkanGPUManager *manager) :
    VulkanObject(manager)
{
    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags =
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
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
 * @return              Allocated command buffer.
 */
VulkanCommandBuffer *VulkanCommandPool::allocateTransient() {
    auto &currentFrame = manager()->currentFrame();

    VulkanCommandBuffer *buffer = new VulkanCommandBuffer(this, true);
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
 * @param transient     Whether the buffer is transient. */
VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandPool *pool, bool transient) :
    VulkanHandle(pool->manager()),
    m_pool(pool),
    m_transient(transient),
    m_state(State::kAllocated)
{
    // TODO: Always transient for now. Fix in destructor as well.
    check(transient);

    // TODO: Secondary command buffers.
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = m_pool->m_transientPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    checkVk(vkAllocateCommandBuffers(manager()->device()->handle(), &allocateInfo, &m_handle));
}

/** Destroy the command buffer. */
VulkanCommandBuffer::~VulkanCommandBuffer() {
    check(m_state != State::kSubmitted);
    vkFreeCommandBuffers(manager()->device()->handle(), m_pool->m_transientPool, 1, &m_handle);
}

/** Begin recording a command buffer.
 * @param usage         Usage flags. */
void VulkanCommandBuffer::begin(VkCommandBufferUsageFlagBits usage) {
    check(m_state == State::kAllocated);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usage;

    checkVk(vkBeginCommandBuffer(m_handle, &beginInfo));
    m_state = State::kRecording;
}

/** Finish recording a command buffer. */
void VulkanCommandBuffer::end() {
    check(m_state == State::kRecording);

    checkVk(vkEndCommandBuffer(m_handle));
    m_state = State::kRecorded;
}
