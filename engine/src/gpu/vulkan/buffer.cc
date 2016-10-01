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

#include "buffer.h"
#include "manager.h"

/** Create a new buffer.
 * @param manager       Manager which owns the buffer.
 * @param desc          Descriptor for the buffer. */
VulkanBuffer::VulkanBuffer(VulkanGPUManager *manager, const GPUBufferDesc &desc) :
    GPUBuffer(desc),
    VulkanObject(manager),
    m_generation(0),
    m_dynamicCount(1),
    m_dynamicIndex(0),
    m_mapSize(0)
{
    /* See description of m_dynamicCount. */
    if (m_type == kUniformBuffer && m_usage == kDynamicUsage)
        m_dynamicCount = kNumPendingFrames;

    /* Allocate the buffer. */
    reallocate();
}

/** Destroy the buffer. */
VulkanBuffer::~VulkanBuffer() {
    manager()->memoryManager()->freeBuffers(m_allocations);
}

/** (Re)allocate the buffer. */
void VulkanBuffer::reallocate() {
    manager()->memoryManager()->freeBuffers(m_allocations);

    /* Determine Vulkan usage flag. */
    VkBufferUsageFlags usageFlag;
    switch (m_type) {
        case kVertexBuffer:
            usageFlag = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case kIndexBuffer:
            usageFlag = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case kUniformBuffer:
            usageFlag = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        default:
            unreachable();
    }

    /* Determine memory flags based on the usage flag given. */
    uint32_t memoryFlags;
    switch (m_usage) {
        case kStaticUsage:
            memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case kDynamicUsage:
        case kTransientUsage:
            memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        default:
            unreachable();
    }

    m_allocations = manager()->memoryManager()->allocateBuffers(
        m_size,
        m_dynamicCount,
        usageFlag,
        memoryFlags);

    m_generation++;
    m_dynamicIndex = 0;
}

/** Map the buffer.
 * @param offset        Offset to map from.
 * @param size          Size of the range to map.
 * @param flags         Bitmask of mapping behaviour flags (see MapFlags).
 * @param access        Access mode.
 * @return              Pointer to mapped buffer. */
void *VulkanBuffer::map(size_t offset, size_t size, uint32_t flags, uint32_t access) {
    check(size);
    check((offset + size) <= m_size);
    check(access == kWriteAccess);

    check(!m_mapSize);

    void *ret;

    if (m_usage == kStaticUsage) {
        /* Allocate a staging buffer. */
        m_mapStaging = manager()->memoryManager()->allocateStagingMemory(size);
        ret = m_mapStaging->map();
    } else {
        checkMsg(
            (flags & kMapInvalidateBuffer) || (offset == 0 && size == m_size),
            "Non-invalidating dynamic buffer mappings not implemented");

        /* We're invalidating the whole buffer, so re-allocate it if it is in
         * use, to save us having to synchronise. */
        if (allocation()->isInUse()) {
            if (m_dynamicCount > 1) {
                /* Advance to the next allocation. */
                m_dynamicIndex = (m_dynamicIndex + 1) % m_dynamicCount;

                /* If the next allocation is still in use, bump up the count
                 * and reallocate. */
                if (allocation()->isInUse()) {
                    m_dynamicCount++;
                    reallocate();
                    logDebug("VulkanBuffer: Bumped allocation count to %zu", m_dynamicCount);
                }
            } else {
                reallocate();
            }
        }

        ret = allocation()->map() + offset;
    }

    m_mapOffset = offset;
    m_mapSize = size;

    return ret;
}

/** Unmap the previous mapping created for the buffer. */
void VulkanBuffer::unmap() {
    check(m_mapSize);

    if (m_usage == kStaticUsage) {
        /* Upload the staging buffer. */
        VkBufferCopy bufferCopy = {};
        bufferCopy.srcOffset = 0;
        bufferCopy.dstOffset = allocation()->offset() + m_mapOffset;
        bufferCopy.size = m_mapSize;

        VulkanCommandBuffer *stagingCmdBuf = manager()->memoryManager()->getStagingCmdBuf();
        vkCmdCopyBuffer(
            stagingCmdBuf->handle(),
            m_mapStaging->buffer(),
            allocation()->buffer(),
            1, &bufferCopy);

        stagingCmdBuf->addReference(this);
    }

    m_mapSize = 0;
}

/** Create a GPU buffer.
 * @param desc          Descriptor for the buffer.
 * @return              Pointer to created buffer. */
GPUBufferPtr VulkanGPUManager::createBuffer(const GPUBufferDesc &desc) {
    return new VulkanBuffer(this, desc);
}
