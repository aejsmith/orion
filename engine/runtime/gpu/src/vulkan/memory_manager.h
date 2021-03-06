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

class VulkanCommandBuffer;
class VulkanDevice;

struct VulkanFrame;

/** Buffer pool allocation size (minimum). */
static const VkDeviceSize kBufferPoolSize = 8 * 1024 * 1024;

/** Image pool allocation size (minimum). */
static const VkDeviceSize kImagePoolSize = 128 * 1024 * 1024;

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
 *
 * We implement different behaviour depending on the usage of a buffer:
 *
 *  - Static:  This indicates that a buffer is long-lived and infrequently
 *             changed. For these, we allocate device-local memory, and use
 *             staging buffers to upload data (more on those below).
 *  - Dynamic: These buffers are for frequently changed data that may be used
 *             across a few frames. For these we allocate host-visible and
 *             coherent memory.
 *
 * Staging buffers are used to upload data for static buffers and for textures.
 * These are allocated as host-visible memory, and allocated as needed rather
 * than from the usual memory pool. We free them once the frame that they were
 * allocated within has completed.
 */
class VulkanMemoryManager : public VulkanObject {
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
    class ResourceMemory : public Refcounted {
    public:
        /** @return             Offset in the parent. */
        VkDeviceSize offset() { return m_parent.second->offset; }
        /** @return             Size of the allocation. */
        VkDeviceSize size() { return m_parent.second->size; }
        /** @return             Handle for the device memory allocation. */
        VkDeviceMemory memory() { return m_parent.first->handle; }

        /** @return             Whether the memory is in use. */
        bool isInUse() const {
            return refcount() > 1;
        }

        /** Get a mapping of the memory (must have been allocated host-visible).
         * @return              Pointer to mapped memory. */
        uint8_t *map() {
            check(m_parent.first->mapping);
            return m_parent.first->mapping + offset();
        }
    protected:
        /** Initialise the handle.
         * @param parent        Reference to the pool that this came from. */
        explicit ResourceMemory(const PoolReference &parent) :
            m_parent(parent)
        {
            /* Reference which is released when freeResource() is called. */
            retain();
        }

        /** Release the memory. */
        void released() override {
            m_parent.first->manager->releaseResource(this);
        }

        PoolReference m_parent;

        friend class VulkanMemoryManager;
    };

    /** Class containing details of a buffer memory allocation. */
    class BufferMemory : public ResourceMemory {
    public:
        /** @return             Handle for the buffer. */
        VkBuffer buffer() { return m_parent.first->buffer; }

        /** Initialise the handle.
         * @param parent        Reference to the pool that this came from. */
        explicit BufferMemory(const PoolReference &parent) :
            ResourceMemory(parent)
        {}
    };

    /** Class containing details of an image memory allocation. */
    class ImageMemory : public ResourceMemory {
    public:
        /** Initialise the handle.
         * @param parent        Reference to the pool that this came from. */
        explicit ImageMemory(const PoolReference &parent) :
            ResourceMemory(parent)
        {}
    };

    /** Class containing details of a staging buffer allocation. */
    class StagingMemory {
    public:
        /** @return             Handle for the buffer. */
        VkBuffer buffer() { return m_buffer; }
        /** @return             Pointer to the staging memory. */
        void *map() { return m_mapping; }
    private:
        VkDeviceMemory m_memory;        /**< Device memory allocation. */
        VkBuffer m_buffer;              /**< Buffer handle. */
        void *m_mapping;                /**< Mapping of the memory. */

        friend class VulkanMemoryManager;
    };

    explicit VulkanMemoryManager(VulkanGPUManager *manager);
    ~VulkanMemoryManager();

    std::vector<BufferMemory *> allocateBuffers(VkDeviceSize size,
                                                size_t count,
                                                VkBufferUsageFlags usage,
                                                VkMemoryPropertyFlags memoryFlags);
    ImageMemory *allocateImage(VkMemoryRequirements &requirements);
    void freeResource(ResourceMemory *memory);

    /** Free buffer allocations returned from allocateBuffers().
     * @param memory        Allocations to free. */
    void freeBuffers(std::vector<BufferMemory *> &memory) {
        for (auto handle : memory)
            freeResource(handle);
        memory.clear();
    }

    StagingMemory *allocateStagingMemory(VkDeviceSize size);

    VulkanCommandBuffer *getStagingCmdBuf();
    void flushStagingCmdBuf();

    void cleanupFrame(VulkanFrame &frame, bool completed);
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
        VulkanMemoryManager *manager;   /**< Manager that this pool belongs to. */

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

    uint32_t selectMemoryType(VkMemoryPropertyFlags flags, uint32_t typeBits = 0xffffffff) const;

    Pool *createPool(VkDeviceSize size, uint32_t memoryType);
    std::vector<PoolReference> allocatePoolEntries(Pool *pool, VkDeviceSize size, size_t count, VkDeviceSize alignment);
    void freePoolEntry(const PoolReference &reference);

    void releaseResource(ResourceMemory *handle);

    /** Device memory properties. */
    VkPhysicalDeviceMemoryProperties m_properties;

    /** Currently existing buffer memory pools. */
    std::list<Pool *> m_bufferPools;
    /** Currently existing image memory pools. */
    std::list<Pool *> m_imagePools;

    /** Command buffer for host to device memory transfers. */
    VulkanCommandBuffer *m_stagingCmdBuf;

    friend class ResourceMemory;
};
