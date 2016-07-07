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

#include "device.h"
#include "memory_manager.h"

/** Initialise the memory manager.
 * @param device        Device that the memory manager is for. */
VulkanMemoryManager::VulkanMemoryManager(VulkanDevice *device) :
    m_device(device)
{
    vkGetPhysicalDeviceMemoryProperties(m_device->physicalHandle(), &m_properties);

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

/** Create a new pool.
 * @param size          Size of the pool to allocate.
 * @param memoryType    Required memory type.
 * @return              Pointer to pool created. */
VulkanMemoryManager::Pool *VulkanMemoryManager::createPool(VkDeviceSize size, uint32_t memoryType) {
    Pool *pool = new Pool();

    /* Allocate a block of device memory. */
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = size;
    allocateInfo.memoryTypeIndex = memoryType;
    checkVk(vkAllocateMemory(m_device->handle(), &allocateInfo, nullptr, &pool->handle));

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
            m_device->handle(),
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
    const VkPhysicalDeviceLimits &limits = m_device->limits();
    VkDeviceSize alignment = 0;
    if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
        alignment = std::max(alignment, limits.minUniformBufferOffsetAlignment);
    if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        alignment = std::max(alignment, limits.minStorageBufferOffsetAlignment);
    if (usage & (VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT))
        alignment = std::max(alignment, limits.minTexelBufferOffsetAlignment);

    /* Select the memory type that we should use. As detailed in section 10.2
     * of the spec, the memory type indices are ordered such that index X <= Y
     * if X's properties are a strict subset of Y's, or if they are the same
     * and X is determined by the implementation to be "better" than Y. */
    uint32_t memoryType = 0;
    while (true) {
        const VkMemoryType &typeInfo = m_properties.memoryTypes[memoryType];

        if ((typeInfo.propertyFlags & memoryFlags) == memoryFlags)
            break;

        if (++memoryType == m_properties.memoryTypeCount)
            fatal("No memory type to satisfy allocation with properties 0x%x", memoryFlags);
    }

    PoolReference reference;
    bool found = false;

    /* Look for an existing pool with free space that we can allocate from. */
    for (Pool *pool : m_bufferPools) {
        if (pool->memoryType != memoryType)
            continue;

        found = allocatePoolEntry(pool, size, alignment, reference);
        if (found) {
            logDebug(
                "VulkanMemoryManager: Allocated from existing pool %p %" PRIu64 " %" PRIu64,
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
        Pool *pool = createPool(std::max(kBufferPoolSize, size), memoryType);

        /* Allocate a buffer object. We mark the buffer as usable for all types
         * of GPUBuffer we can create, as we mix buffer types within a pool. */
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = pool->size;
        bufferCreateInfo.usage =
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        checkVk(vkCreateBuffer(m_device->handle(), &bufferCreateInfo, nullptr, &pool->buffer));

        /* Bind the memory to the buffer. */
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(m_device->handle(), pool->buffer, &requirements);
        check(requirements.size == pool->size);
        check(requirements.memoryTypeBits & (1 << memoryType));
        checkVk(vkBindBufferMemory(m_device->handle(), pool->buffer, pool->handle, 0));

        m_bufferPools.push_back(pool);

        /* Allocate the entry. This should always succeed. */
        found = allocatePoolEntry(pool, size, alignment, reference);
        check(found);

        logDebug(
            "VulkanMemoryManager: Allocated new pool %p %" PRIu64 " %" PRIu64,
            pool, reference.second->offset, reference.second->size);
    }

    auto handle = new BufferMemory(reference);
    reference.second->child = handle;
    return handle;
}

/** Free a buffer memory allocation.
 * @param allocation    Handle returned from allocateBuffer(). */
void VulkanMemoryManager::freeBuffer(BufferMemory *suballocation) {
    fatal("VulkanMemoryManager::freeBuffer: TODO");
}
