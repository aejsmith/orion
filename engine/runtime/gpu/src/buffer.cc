/*
 * Copyright (C) 2015-2016 Alex Smith
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
 * @brief               GPU buffer class.
 */

#include "gpu/buffer.h"

/** Construct the GPU buffer.
 * @param desc          Descriptor for the buffer. */
GPUBuffer::GPUBuffer(const GPUBufferDesc &desc) :
    m_type(desc.type),
    m_usage(desc.usage),
    m_size(desc.size)
{}

/**
 * Write data to the buffer.
 *
 * Replaces some or all of the current buffer content with new data. The area
 * to write must lie within the bounds of the buffer, i.e. (offset + size) must
 * be less than or equal to the buffer size.
 *
 * @param offset        Offset to write at.
 * @param size          Size of the data to write.
 * @param buf           Buffer containing data to write.
 * @param flags         Mapping flags to use.
 */
void GPUBuffer::write(size_t offset, size_t size, const void *buf, uint32_t flags) {
    checkMsg((offset + size) <= m_size,
             "Write outside buffer bounds (total: %zu, offset: %zu, size: %zu)",
             m_size, offset, size);

    void *data = map(offset, size, kWriteAccess, flags);
    memcpy(data, buf, size);
    unmap();
}
