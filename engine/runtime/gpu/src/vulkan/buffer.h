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
 * @brief               Vulkan buffer implementation.
 */

#pragma once

#include "memory_manager.h"

/** Vulkan GPU buffer implementation. */
class VulkanBuffer : public GPUBuffer, public VulkanObject {
public:
    VulkanBuffer(VulkanGPUManager *manager, const GPUBufferDesc &desc);

    void *map(size_t offset, size_t size, uint32_t flags, uint32_t access) override;
    void unmap() override;

    /** @return             Memory allocation currently backing this buffer. */
    VulkanMemoryManager::BufferMemory *allocation() const { return m_allocations[m_dynamicIndex]; }
    /** @return             Generation number for tracking reallocations. */
    uint32_t generation() const { return m_generation; }
protected:
    ~VulkanBuffer();
private:
    void reallocate();

    /** Memory allocations backing this buffer. */
    std::vector<VulkanMemoryManager::BufferMemory *> m_allocations;

    /**
     * Generation number.
     *
     * This is used to keep track of when a buffer is reallocated. Each time a
     * reallocation occurs this number is increased. The reason we do this
     * instead of just using the allocation handle pointer is that theoretically
     * it is possible for one handle to be freed then another to be allocated at
     * the same address. We could add a reference to the handle when we're using
     * it for tracking but that would possibly prevent the allocation from being
     * freed.
     */
    uint32_t m_generation;

    /**
     * Total number of dynamic allocations.
     *
     * Dynamic uniform buffers have this number of allocations created up front,
     * and we cycle between each allocation when invalidating the old contents
     * instead of creating a whole new allocation. We ask the memory manager to
     * allocate all of these allocations on the same VkBuffer object. All of
     * this allows us to use dynamic offsets for uniform buffer bindings and
     * never have to change the descriptor because the buffer will always remain
     * on the same VkBuffer. This value defaults to kNumPendingFrames which
     * is sufficient for a buffer which is reallocated once per frame, but if we
     * reallocate more frequently than this then we increase this value.
     */
    uint32_t m_dynamicCount;

    /** Current dynamic buffer index (see m_dynamicCount). */
    uint32_t m_dynamicIndex;

    size_t m_mapOffset;             /**< Current mapping offset. */
    size_t m_mapSize;               /**< Current mapping size. */

    /** Staging memory for the current mapping (for static buffers). */
    VulkanMemoryManager::StagingMemory *m_mapStaging;
};
