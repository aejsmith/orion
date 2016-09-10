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
#include "device.h"

/** Create a new buffer.
 * @param manager       Manager which owns the buffer.
 * @param type          Type of the buffer.
 * @param usage         Usage hint.
 * @param size          Buffer size. */
VulkanBuffer::VulkanBuffer(VulkanGPUManager *manager, Type type, Usage usage, size_t size) :
    GPUBuffer(type, usage, size),
    VulkanObject(manager)
{
    /* Determine Vulkan usage flag. */
    VkBufferUsageFlags usageFlag;
    switch (type) {
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
            check(false);
    }

    // FIXME: Staging buffers! Dynamic/per-frame need to handle differently too.
    // Staging buffers must be created with VK_BUFFER_USAGE_TRANSFER_SRC_BIT.
    m_allocation = manager->memoryManager()->allocateBuffer(
        m_size,
        usageFlag,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

/** Destroy the buffer. */
VulkanBuffer::~VulkanBuffer() {
    manager()->memoryManager()->freeBuffer(m_allocation);
}

/** Write data to the buffer.
 * @param offset        Offset to write at.
 * @param size          Size of the data to write.
 * @param buf           Buffer containing data to write. */
void VulkanBuffer::writeImpl(size_t offset, size_t size, const void *buf) {
    // FIXME: This is all wrong for buffers that might be already in use on the
    // GPU. Need to wait until the access has completed. We really need staging
    // buffers here. This is just to get things going.

    uint8_t *mapping = m_allocation->map();
    std::memcpy(mapping + offset, buf, size);
}

/** Map the buffer.
 * @param offset        Offset to map from.
 * @param size          Size of the range to map.
 * @param flags         Bitmask of mapping behaviour flags (see MapFlags).
 * @param access        Bitmask of access flags.
 * @return              Pointer to mapped buffer. */
void *VulkanBuffer::mapImpl(size_t offset, size_t size, uint32_t flags, uint32_t access) {
    fatal("VulkanBuffer::mapImpl: TODO");
}

/** Unmap the previous mapping created for the buffer. */
void VulkanBuffer::unmapImpl() {
    fatal("VulkanBuffer::unmapImpl: TODO");
}

/** Create a GPU buffer.
 * @param type          Type of the buffer.
 * @param usage         Usage hint.
 * @param size          Buffer size.
 * @return              Pointer to created buffer. */
GPUBufferPtr VulkanGPUManager::createBuffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) {
    return new VulkanBuffer(this, type, usage, size);
}
