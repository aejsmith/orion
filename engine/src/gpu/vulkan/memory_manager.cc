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

#include "manager.h"
#include "memory_manager.h"

/* Set to 1 to enable debug messages. */
#define MEMORY_DEBUG 0
#if MEMORY_DEBUG
#   define logMemoryDebug(fmt...) logDebug(fmt)
#else
#   define logMemoryDebug(fmt...)
#endif

/** Initialise the memory manager.
 * @param manager       Manager that the memory manager is for. */
VulkanMemoryManager::VulkanMemoryManager(VulkanGPUManager *manager) :
    VulkanObject(manager),
    m_stagingCmdBuf(nullptr)
{
    vkGetPhysicalDeviceMemoryProperties(manager->device()->physicalHandle(), &m_properties);

    logInfo("  Memory Heaps:");

    for (uint32_t i = 0; i < m_properties.memoryHeapCount; i++) {
        const VkMemoryHeap &heap = m_properties.memoryHeaps[i];

        auto addFlag =
            [] (std::string &str, const char *flag) {
                str += (str.empty()) ? " = " : ", ";
                str += flag;
            };

        std::string heapFlags;
        if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            addFlag(heapFlags, "device local");

        logInfo(
            "    Heap %u: %" PRIu64 " bytes / %" PRIu64 " MB, 0x%x%s",
            i, heap.size, heap.size / 1024 / 1024, heap.flags, heapFlags.c_str());

        for (uint32_t j = 0; j < m_properties.memoryTypeCount; j++) {
            const VkMemoryType &type = m_properties.memoryTypes[j];

            if (type.heapIndex != i || !type.propertyFlags)
                continue;

            std::string typeFlags;

            if (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                addFlag(typeFlags, "device local");
            if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                addFlag(typeFlags, "visible");
            if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                addFlag(typeFlags, "coherent");
            if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
                addFlag(typeFlags, "cached");
            if (type.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
                addFlag(typeFlags, "lazy");

            logInfo("      Type %u: 0x%x%s", j, type.propertyFlags, typeFlags.c_str());
        }
    }
}

/** Shut down the memory manager. */
VulkanMemoryManager::~VulkanMemoryManager() {}

/** Select a memory type which supports the given flags.
 * @param flags         Memory property flags.
 * @param typeBits      Allowed memory type bits.
 * @return              Selected memory type. */
uint32_t VulkanMemoryManager::selectMemoryType(VkMemoryPropertyFlags flags, uint32_t typeBits) const {
    /* As detailed in section 10.2 of the spec, the memory type indices are
     * ordered such that index X <= Y if X's properties are a strict subset of
     * Y's, or if they are the same and X is determined by the implementation to
     * be "better" than Y. */
    for (uint32_t memoryType = 0; memoryType < m_properties.memoryTypeCount; memoryType++) {
        if (typeBits & (1 << memoryType)) {
            const VkMemoryType &typeInfo = m_properties.memoryTypes[memoryType];

            if (typeInfo.propertyFlags && (typeInfo.propertyFlags & flags) == flags)
                return memoryType;
        }
    }

    fatal("No memory type to satisfy allocation with properties 0x%x, types 0x%x", flags, typeBits);
}

/** Create a new pool.
 * @param size          Size of the pool to allocate.
 * @param memoryType    Required memory type.
 * @return              Pointer to pool created. */
VulkanMemoryManager::Pool *VulkanMemoryManager::createPool(VkDeviceSize size, uint32_t memoryType) {
    auto pool = new Pool();

    /* Allocate a block of device memory. */
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = size;
    allocateInfo.memoryTypeIndex = memoryType;
    checkVk(vkAllocateMemory(manager()->device()->handle(), &allocateInfo, nullptr, &pool->handle));

    pool->buffer = VK_NULL_HANDLE;
    pool->size = size;
    pool->memoryType = memoryType;

    /* Create an initial free entry covering the entire allocation. */
    pool->entries.emplace_back();
    auto entry = pool->entries.begin();
    entry->offset = 0;
    entry->size = size;
    entry->child = nullptr;
    pool->freeEntries.push_back(entry);

    if (m_properties.memoryTypes[memoryType].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        checkVk(vkMapMemory(
            manager()->device()->handle(),
            pool->handle,
            0, pool->size,
            0,
            reinterpret_cast<void **>(&pool->mapping)));
    } else {
        pool->mapping = nullptr;
    }

    return pool;
}

/** Allocate an entry from a pool.
 * @param pool          Pool to allocate from.
 * @param size          Allocation size.
 * @param alignment     Required alignment.
 * @param reference     Where to store pool entry reference. The entry is not
 *                      marked as allocated on success, a ResourceMemory object
 *                      should be allocated for it and set.
 * @return              Whether an entry was successfully allocated. */
bool VulkanMemoryManager::allocatePoolEntry(
    Pool *pool,
    VkDeviceSize size,
    VkDeviceSize alignment,
    PoolReference &reference)
{
    /* Look for a free entry. */
    for (auto free = pool->freeEntries.begin(); free != pool->freeEntries.end(); ++free) {
        /* The free list entry is an iterator referring to the main entry
         * list. This allows us to quickly modify that list. */
        std::list<PoolEntry>::iterator entry = *free;

        check(!entry->child);

        /* See if this entry can satisfy the alignment constraints. */
        VkDeviceSize alignedOffset = (alignment)
            ? Math::roundUp(entry->offset, alignment)
            : entry->offset;
        VkDeviceSize diff = alignedOffset - entry->offset;
        if (diff > entry->size || entry->size - diff < size)
            continue;

        /* We can now remove this entry from the free list. Invalidates the
         * iterator but we aren't going to continue. */
        pool->freeEntries.erase(free);

        /* If alignment caused a difference in the offset, we have to split
         * the entry. */
        if (diff) {
            auto split = pool->entries.emplace(entry);
            split->offset = entry->offset;
            split->size = diff;
            split->child = nullptr;

            /* This split is likely to be small as it was only created due
             * to alignment. Push it back on the end of this list so that
             * we vaguely try to keep larger entries towards the front. */
            pool->freeEntries.push_back(split);

            entry->offset += diff;
            entry->size -= diff;
        }

        /* If the entry is bigger than requested, split it. */
        if (entry->size > size) {
            auto split = pool->entries.emplace(std::next(entry));
            split->offset = entry->offset + size;
            split->size = entry->size - size;
            split->child = nullptr;

            pool->freeEntries.push_front(split);

            entry->size = size;
        }

        reference = std::make_pair(pool, entry);
        return true;
    }

    return false;
}

/** Free a pool entry.
 * @param reference     Pool reference. */
void VulkanMemoryManager::freePoolEntry(const PoolReference &reference) {
    Pool *pool = reference.first;
    std::list<PoolEntry>::iterator entry = reference.second;

    entry->child = nullptr;

    logMemoryDebug(
        "VulkanMemoryManager: Freed allocation from pool %p %" PRIu64 " %" PRIu64,
        pool, entry->offset, entry->size);

    /* Check if we can merge this entry with the previous one. */
    if (entry != pool->entries.begin()) {
        auto prev = std::prev(entry);

        if (!prev->child) {
            entry->offset = prev->offset;
            entry->size += prev->size;
            pool->freeEntries.remove(prev);
            pool->entries.erase(prev);

            logMemoryDebug(
                "VulkanMemoryManager: Merged with previous %" PRIu64 " %" PRIu64,
                entry->offset, entry->size);
        }
    }

    /* Same for the following one. */
    if (entry != pool->entries.begin()) {
        auto next = std::next(entry);

        if (!next->child) {
            entry->size += next->size;
            pool->freeEntries.remove(next);
            pool->entries.erase(next);

            logMemoryDebug(
                "VulkanMemoryManager: Merged with next %" PRIu64 " %" PRIu64,
                entry->offset, entry->size);
        }
    }

    /* Push it onto the free list. */
    pool->freeEntries.push_front(entry);
}

/**
 * Allocate memory for a buffer.
 *
 * This function allocates memory to back a buffer. The memory returned is a
 * suballocation of a potentially larger allocation. A single VkBuffer object
 * is created covering the entire large allocation, the suballocation is
 * given an offset within that. Therefore, the user of this suballocation must
 * use both the given buffer object and the offset to refer to it.
 *
 * @param size          Allocation size.
 * @param usage         Buffer usage flags.
 * @param memoryFlags   Requested memory property flags.
 *
 * @return              Handle to the allocated memory.
 */
VulkanMemoryManager::BufferMemory *VulkanMemoryManager::allocateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags)
{
    /* From the usage given, determine the required alignment of the buffer. */
    const VkPhysicalDeviceLimits &limits = manager()->device()->limits();
    VkDeviceSize alignment = 0;
    if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
        alignment = std::max(alignment, limits.minUniformBufferOffsetAlignment);
    if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        alignment = std::max(alignment, limits.minStorageBufferOffsetAlignment);
    if (usage & (VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT))
        alignment = std::max(alignment, limits.minTexelBufferOffsetAlignment);

    /* Select the memory type that we should use. */
    uint32_t memoryType = selectMemoryType(memoryFlags);

    PoolReference reference;
    bool found = false;

    /* Look for an existing pool with free space that we can allocate from. */
    for (Pool *pool : m_bufferPools) {
        if (pool->memoryType != memoryType)
            continue;

        found = allocatePoolEntry(pool, size, alignment, reference);
        if (found) {
            logMemoryDebug(
                "VulkanMemoryManager: Allocated buffer from existing pool %p %" PRIu64 " %" PRIu64,
                pool, reference.second->offset, reference.second->size);
            break;
        }
    }

    /* If nothing is found, create a new pool. */
    if (!found) {
        /* In case the allocation size is larger than our standard pool size,
         * take the maximum. Note that vkAllocateMemory() is guaranteed to
         * return memory that can satisfy all alignment requirements of the
         * implementation. */
        auto pool = createPool(std::max(kBufferPoolSize, size), memoryType);

        VkDevice device = manager()->device()->handle();

        /* Allocate a buffer object. */
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = pool->size;

        /* We mark the buffer as usable for all types of GPUBuffer we can
         * create, as we mix buffer types within a pool. */
        bufferCreateInfo.usage =
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        /* If this is device local, we probably want to be able to transfer to it. */
        if (memoryFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        checkVk(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &pool->buffer));

        /* Bind the memory to the buffer. */
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(device, pool->buffer, &requirements);
        check(requirements.size == pool->size);
        check(requirements.memoryTypeBits & (1 << memoryType));
        checkVk(vkBindBufferMemory(device, pool->buffer, pool->handle, 0));

        m_bufferPools.push_back(pool);

        /* Allocate the entry. This should always succeed. */
        found = allocatePoolEntry(pool, size, alignment, reference);
        check(found);

        logMemoryDebug(
            "VulkanMemoryManager: Allocated new buffer pool %p %" PRIu64 " %" PRIu64,
            pool, reference.second->offset, reference.second->size);
    }

    auto handle = new BufferMemory(reference);
    reference.second->child = handle;
    return handle;
}

/**
 * Allocate memory for an image.
 *
 * This function allocates memory to back an image. The memory returned is a
 * suballocation of a potentially larger allocation.
 *
 * @param requirements  Image memory requirements.
 *
 * @return              Handle to the allocated memory.
 */
VulkanMemoryManager::ImageMemory *VulkanMemoryManager::allocateImage(VkMemoryRequirements &requirements) {
    /* Select a memory type. */
    uint32_t memoryType = selectMemoryType(0, requirements.memoryTypeBits);

    PoolReference reference;
    bool found = false;

    /* Look for an existing pool with free space that we can allocate from. */
    for (Pool *pool : m_imagePools) {
        if (pool->memoryType != memoryType)
            continue;

        found = allocatePoolEntry(pool, requirements.size, requirements.alignment, reference);
        if (found) {
            logMemoryDebug(
                "VulkanMemoryManager: Allocated image from existing pool %p %" PRIu64 " %" PRIu64,
                pool, reference.second->offset, reference.second->size);
            break;
        }
    }


    /* If nothing is found, create a new pool. */
    if (!found) {
        /* In case the allocation size is larger than our standard pool size,
         * take the maximum. */
        auto pool = createPool(std::max(kImagePoolSize, requirements.size), memoryType);
        m_imagePools.push_back(pool);

        /* Allocate the entry. This should always succeed. */
        found = allocatePoolEntry(pool, requirements.size, requirements.alignment, reference);
        check(found);

        logMemoryDebug(
            "VulkanMemoryManager: Allocated new image pool %p %" PRIu64 " %" PRIu64,
            pool, reference.second->offset, reference.second->size);
    }

    auto handle = new ImageMemory(reference);
    reference.second->child = handle;
    return handle;
}

/** Free a resource memory allocation.
 * @param handle        Handle returned from allocateBuffer() or allocateImage(). */
void VulkanMemoryManager::freeResource(ResourceMemory *handle) {
    /* Just remove the reference we added to it when we first allocated it.
     * Any command buffers using the memory will still hold a reference, so we
     * will not actually free the memory until it is no longer in use. This is
     * done by releaseResource(). */
    handle->release();
}

/** Actually free resource memory that is no longer in use.
 * @param handle        Handle to memory to free. */
void VulkanMemoryManager::releaseResource(ResourceMemory *handle) {
    freePoolEntry(handle->m_parent);
    delete handle;
}

/**
 * Allocate staging memory.
 *
 * This function allocates a block of memory visible to the host for use as a
 * staging buffer to transfer to device local memory. The buffer will be added
 * to a list of buffers to be freed once the current frame is finished on the
 * GPU, therefore should not be used across multiple frames.
 *
 * @param size          Size of the buffer to allocate.
 *
 * @return              Staging memory handle.
 */
VulkanMemoryManager::StagingMemory *VulkanMemoryManager::allocateStagingMemory(VkDeviceSize size) {
    VkDevice device = manager()->device()->handle();

    auto memory = new StagingMemory();

    /* Staging memory should be host visible and coherent. */
    uint32_t memoryType = selectMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    /* Allocate a buffer object. */
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    checkVk(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &memory->m_buffer));

    /* Allocate a device memory block. */
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(device, memory->m_buffer, &requirements);
    check(requirements.memoryTypeBits & (1 << memoryType));
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = memoryType;
    checkVk(vkAllocateMemory(device, &allocateInfo, nullptr, &memory->m_memory));

    /* Bind memory to the buffer. */
    checkVk(vkBindBufferMemory(device, memory->m_buffer, memory->m_memory, 0));

    /* And finally map it. */
    checkVk(vkMapMemory(device, memory->m_memory, 0, requirements.size, 0, &memory->m_mapping));

    /* Record it to be freed at the end of the frame. */
    manager()->currentFrame().stagingAllocations.push_back(memory);

    return memory;
}

/** Free up previous frame memory allocations.
 * @param frame         Frame to clean up.
 * @param completed     Whether the frame has completed. */
void VulkanMemoryManager::cleanupFrame(VulkanFrame &frame, bool completed) {
    if (!completed)
        return;

    VkDevice device = manager()->device()->handle();

    /* Free staging allocations. */
    while (!frame.stagingAllocations.empty()) {
        StagingMemory *memory = frame.stagingAllocations.front();
        frame.stagingAllocations.pop_front();

        vkUnmapMemory(device, memory->m_memory);
        vkDestroyBuffer(device, memory->m_buffer, nullptr);
        vkFreeMemory(device, memory->m_memory, nullptr);

        delete memory;
    }
}

/**
 * Get a command buffer for staging transfers.
 *
 * Gets a command buffer to be used for host to device-local memory transfers.
 * This will be flushed prior to submission of any other commands.
 *
 * @return              Command buffer for staging transfers.
 */
VulkanCommandBuffer *VulkanMemoryManager::getStagingCmdBuf() {
    if (!m_stagingCmdBuf) {
        m_stagingCmdBuf = manager()->commandPool()->allocateTransient();
        m_stagingCmdBuf->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    }

    return m_stagingCmdBuf;
}

/** Submit the staging command buffer. */
void VulkanMemoryManager::flushStagingCmdBuf() {
    if (m_stagingCmdBuf) {
        // TODO: Could use a separate transfer queue here?
        // TODO: If we submit all frame work in a single vkQueueSubmit at the
        // end of a frame, perhaps we could bundle this into the same call?
        m_stagingCmdBuf->end();
        manager()->queue()->submit(m_stagingCmdBuf);

        /* Will be freed with the frame, it's transient. */
        m_stagingCmdBuf = nullptr;
    }
}
