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
 */

#include "command_buffer.h"

/** Create a command pool.
 * @param device        Device that this command pool belongs to.
 * @param queueFamily   Queue family to create command buffers for. */
VulkanCommandPool::VulkanCommandPool(VulkanDevice *device, uint32_t queueFamily) :
    m_device(device)
{
    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags =
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = queueFamily;

    checkVk(vkCreateCommandPool(m_device->handle(), &createInfo, nullptr, &m_transientPool));
}

/** Destroy the command pool. */
VulkanCommandPool::~VulkanCommandPool() {
    vkDestroyCommandPool(m_device->handle(), m_transientPool, nullptr);
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
    check(!m_frames.empty());
    Frame &currentFrame = m_frames.back();

    VulkanCommandBuffer *buffer = new VulkanCommandBuffer(this, true);
    currentFrame.buffers.push_back(buffer);
    return buffer;
}

/** Start a new frame. */
void VulkanCommandPool::startFrame() {
    /* Clean up completed frames. */
    for (auto i = m_frames.begin(); i != m_frames.end(); ) {
        Frame &frame = *i;

        /* Check whether the frame has completed. */
        bool completed = frame.fence.getStatus();

        for (auto j = frame.buffers.begin(); j != frame.buffers.end(); ) {
            VulkanCommandBuffer *buffer = *j;

            /* Free unsubmitted buffers or all buffers if completed. */
            if (buffer->m_state != VulkanCommandBuffer::State::kSubmitted || completed) {
                if (buffer->m_state == VulkanCommandBuffer::State::kSubmitted)
                    buffer->m_state = VulkanCommandBuffer::State::kAllocated;

                delete buffer;
                frame.buffers.erase(j++);
            } else {
                ++j;
            }
        }

        if (frame.buffers.empty()) {
            m_frames.erase(i++);
        } else {
            ++i;
        }
    }

    /* Start the new frame. */
    m_frames.emplace_back(m_device);
}

/** Create a new command buffer.
 * @param pool          Pool the command buffer is being allocated from.
 * @param transient     Whether the buffer is transient. */
VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandPool *pool, bool transient) :
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

    checkVk(vkAllocateCommandBuffers(m_pool->m_device->handle(), &allocateInfo, &m_handle));
}

/** Destroy the command buffer. */
VulkanCommandBuffer::~VulkanCommandBuffer() {
    check(m_state != State::kSubmitted);
    vkFreeCommandBuffers(m_pool->m_device->handle(), m_pool->m_transientPool, 1, &m_handle);
}
