/*
 * Copyright (C) 2015 Alex Smith
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
 * @brief               Vertex format class.
 */

#include "gpu/vertex_format.h"

/** Initialize a vertex format descriptor.
 * @param buffers       Array of buffer layout descriptors. Array is invalidated.
 * @param attributes    Array of attribute descriptors. Array is invalidated. */
GPUVertexFormat::GPUVertexFormat(VertexBufferLayoutArray &buffers, VertexAttributeArray &attributes) :
    m_buffers(std::move(buffers)),
    m_attributes(std::move(attributes))
{
    for (size_t i = 0; i < m_buffers.size(); i++)
        check(m_buffers[i].stride);

    for (size_t i = 0; i < m_attributes.size(); i++) {
        const VertexAttribute &attribute = m_attributes[i];

        check(attribute.count);
        checkMsg(
            attribute.count >= 1 && attribute.count <= 4,
            "Attribute %u vector size %u unsupported", i, attribute.count);
        checkMsg(
            attribute.buffer < m_buffers.size(),
            "Attribute %u references unknown buffer %u", i, attribute.buffer);
        checkMsg(
            (attribute.offset + attribute.size()) <= m_buffers[attribute.buffer].stride,
            "Attribute %u position exceeds buffer stride (offset: %u, size: %u, stride: %u)",
            i, attribute.offset, attribute.size(), m_buffers[attribute.buffer].stride);

        for (size_t j = 0; j < i; j++) {
            const VertexAttribute &other = m_attributes[j];

            checkMsg(
                other.semantic != attribute.semantic || other.index != attribute.index,
                "Attribute %u is semantic duplicate of %u", i, j);
        }
    }
}
