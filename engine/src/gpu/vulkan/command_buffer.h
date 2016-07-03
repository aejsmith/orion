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

#pragma once

#include "device.h"
#include "util.h"

#include <list>

class VulkanCommandBuffer;

/**
 * Class managing a pool of command buffers.
 *
 * This class wraps a Vulkan command buffer pool, and on top of that handles
 * the destruction of buffers when they are no longer needed.
 */
class VulkanCommandPool {
public:
    VulkanCommandPool(VulkanDevice *device, uint32_t queueFamily);
    ~VulkanCommandPool();

    VulkanCommandBuffer *allocateTransient();

    void startFrame();
private:
    /** Structure tracking a single frame's transient buffers. */
    struct Frame {
        /** List of buffers allocated for the frame. */
        std::list<VulkanCommandBuffer *> buffers;
        /** Fence signalled upon completion of the frame's submission. */
        VulkanFence fence;

        Frame(VulkanDevice *device) : fence(device) {}
    };

    VulkanDevice *m_device;             /**< Device that this command pool belongs to. */
    VkCommandPool m_transientPool;      /**< Pool for transient command buffers. */

    /**
     * List of frame data.
     *
     * The current frame's data is the last element of the list. We have to
     * keep around earlier frames' buffers until their work has been completed,
     * which is determined using the fence. Once a frame has been completed, we
     * clean up its buffers and remove it from this list.
     */
    std::list<Frame> m_frames;

    friend class VulkanCommandBuffer;
};

/** Class wrapping a command buffer. */
class VulkanCommandBuffer {
public:
    /** @return             Handle to the command buffer. */
    VkCommandBuffer handle() const { return m_handle; }
private:
    /** State of the command buffer. */
    enum class State {
        kAllocated,                     /**< Allocated but not submitted. */
        kSubmitted,                     /**< Submitted. */
    };

    VulkanCommandBuffer(VulkanCommandPool *pool, bool transient);
    ~VulkanCommandBuffer();

    VulkanCommandPool *m_pool;          /**< Pool that the buffer belongs to. */
    bool m_transient;                   /**< Whether the buffer is transient. */
    VkCommandBuffer m_handle;           /**< Handle to the command buffer. */
    State m_state;                      /**< State of the command buffer. */

    friend class VulkanCommandPool;
};
