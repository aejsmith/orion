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

#include "memory_manager.h"
#include "utility.h"

class VulkanCommandBuffer;

/**
 * Class managing a pool of command buffers.
 *
 * This class wraps a Vulkan command buffer pool, and on top of that handles
 * the destruction of buffers when they are no longer needed.
 */
class VulkanCommandPool : public VulkanObject {
public:
    explicit VulkanCommandPool(VulkanGPUManager *manager);
    ~VulkanCommandPool();

    VulkanCommandBuffer *allocateTransient(VkCommandBufferLevel level);

    void cleanupFrame(VulkanFrame &frame, bool completed);
private:
    VkCommandPool m_transientPool;      /**< Pool for transient command buffers. */

    friend class VulkanCommandBuffer;
};

/** Class wrapping a command buffer. */
class VulkanCommandBuffer : public VulkanHandle<VkCommandBuffer> {
public:
    void begin(
        VkCommandBufferUsageFlags usage,
        const VkCommandBufferInheritanceInfo *inheritance = nullptr);
    void end();

    void addReference(Refcounted *object);
    void addReference(GPUBuffer *buffer);
private:
    /** State of the command buffer. */
    enum class State {
        kAllocated,                     /**< Allocated but not submitted. */
        kRecording,                     /**< Between begin() and end(). */
        kRecorded,                      /**< After end(). */
        kSubmitted,                     /**< Submitted. */
    };

    VulkanCommandBuffer(VulkanCommandPool *pool, VkCommandBufferLevel level, bool transient);
    ~VulkanCommandBuffer();

    VulkanCommandPool *m_pool;          /**< Pool that the buffer belongs to. */
    bool m_transient;                   /**< Whether the buffer is transient. */
    State m_state;                      /**< State of the command buffer. */

    /**
     * List of object references.
     *
     * This is used to record objects which must be kept alive until the command
     * buffer has completed. We just add an extra reference on them which
     * prevents them from being freed.
     */
    std::list<ReferencePtr<Refcounted>> m_references;

    friend class VulkanCommandList;
    friend class VulkanCommandPool;
    friend class VulkanQueue;
};
