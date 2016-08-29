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
 * @brief               Vulkan memory manager.
 */

#pragma once

#include "vulkan.h"

#include <list>

class VulkanDevice;

/** Buffer pool allocation size (minimum). */
static const VkDeviceSize kBufferPoolSize = 8 * 1024 * 1024;

/**
 * Class managing memory for a Vulkan device.
 *
 * This class manages memory allocations for resources (buffers and images).
 * Vulkan organises memory into heaps, which can be device local (visible only
 * to the GPU) or visible to both the host and the GPU. Each heap supports a
 * set of memory types with different properties (e.g. whether coherent with
 * the host). Resources are initially not associated with any memory. Device
 * memory must be allocated from a heap and associated with the resource.
 *
 * While the simplest solution is to perform a device memory allocation to back
 * every individual resource, this is inefficient. Some OSes have a linear cost
 * for the number of allocations involved in each submission to a queue, and
 * there is also a limit on the number of allocations we can perform.
 *
 * Instead, we perform large allocations of device memory, and suballocate this
 * ourselves to individual resources. For buffers, we create a single VkBuffer
 * for each allocation, and then just make use of offsets into that buffer for
 * individual GPUBuffer objects.
 */
class VulkanMemoryManager {
private:
    struct PoolEntry;
    struct Pool;

    /**
     * Reference back to the pool.
     *
     * This allows us to get from a ResourceMemory object back to the Pool it
     * was created from and the PoolEntry that refers to it quickly. The whole
     * purpose of this is to avoid exposing the memory manager implementation
     * details to its users.
     */
    using PoolReference = std::pair<Pool *, std::list<PoolEntry>::iterator>;
public:
    /** Class containing details of a resource memory allocation. */
    class ResourceMemory {
    public:
        /** @return             Offset in the parent. */
        VkDeviceSize offset() const { return m_parent.second->offset; }
        /** @return             Size of the allocation. */
        VkDeviceSize size() const { return m_parent.second->size; }

        /** Get a mapping of the memory (must have been allocated host-visible).
         * @return              Pointer to mapped memory. */
        uint8_t *map() const {
            check(m_parent.first->mapping);
            return m_parent.first->mapping + offset();
        }
    protected:
        ResourceMemory(const PoolReference &parent) :
            m_parent(parent)
        {}

        PoolReference m_parent;
    };

    /** Class containing details of a buffer memory allocation. */
    class BufferMemory : public ResourceMemory {
    public:
        /** @return             Handle for the buffer. */
        VkBuffer handle() const {
            return m_parent.first->buffer;
        }

        BufferMemory(const PoolReference &parent) :
            ResourceMemory(parent)
        {}
    };

    explicit VulkanMemoryManager(VulkanDevice *device);
    ~VulkanMemoryManager();

    BufferMemory *allocateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memoryFlags);
    void freeBuffer(BufferMemory *allocation);
private:
    /** Memory pool suballocation list entry. */
    struct PoolEntry {
        VkDeviceSize offset;            /**< Offset of the suballocation. */
        VkDeviceSize size;              /**< Size of the suballocation. */

        /** Pointer to the child resource (null if free). */
        ResourceMemory *child;
    };

    /** Structure containing details of a device memory pool. */
    struct Pool {
        VkDeviceMemory handle;          /**< Handle to the allocation. */
        VkBuffer buffer;                /**< Buffer handle (if this is a buffer). */
        VkDeviceSize size;              /**< Size of the allocation. */
        uint32_t memoryType;            /**< Memory type index. */
        uint8_t *mapping;               /**< Mapping (for host visible memory, null otherwise). */

        /** Sorted list of entries in the pool (free and non-free). */
        std::list<PoolEntry> entries;

        /** List of references to free pool entries. */
        std::list<std::list<PoolEntry>::iterator> freeEntries;
    };

    Pool *createPool(VkDeviceSize size, uint32_t memoryType);
    bool allocatePoolEntry(Pool *pool, VkDeviceSize size, VkDeviceSize alignment, PoolReference &reference);

    VulkanDevice *m_device;             /**< Device that this memory manager is for. */

    /** Device memory properties. */
    VkPhysicalDeviceMemoryProperties m_properties;

    /** Currently existing buffer memory pools. */
    std::list<Pool *> m_bufferPools;
};
