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
    VulkanBuffer(VulkanGPUManager *manager, Type type, Usage usage, size_t size);
    ~VulkanBuffer();

    void *map(size_t offset, size_t size, uint32_t flags, uint32_t access) override;
    void unmap() override;

    /** @return             Memory allocation currently backing this buffer. */
    VulkanMemoryManager::BufferMemory *allocation() const { return m_allocation; }
private:
    void reallocate();

    /** Memory allocation backing this buffer. */
    VulkanMemoryManager::BufferMemory *m_allocation;

    size_t m_mapOffset;             /**< Current mapping offset. */
    size_t m_mapSize;               /**< Current mapping size. */

    /** Staging memory for the current mapping (for static buffers). */
    VulkanMemoryManager::StagingMemory *m_mapStaging;
};
