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
 * @brief               Vertex data class.
 */

#include "gpu/vertex_data.h"

/** Initialize a vertex data layout object.
 * @param desc          Layout descriptor. */
GPUVertexDataLayout::GPUVertexDataLayout(GPUVertexDataLayoutDesc &&desc) :
    GPUState(std::move(desc))
{
    for (size_t i = 0; i < m_desc.bindings.size(); i++) {
        const VertexBinding &binding = m_desc.bindings[i];

        checkMsg(
            binding.stride,
            "Binding %zu has zero stride", i);
    }

    for (size_t i = 0; i < m_desc.attributes.size(); i++) {
        const VertexAttribute &attribute = m_desc.attributes[i];

        checkMsg(
            attribute.components,
            "Attribute %zu has zero component count", i);
        checkMsg(
            attribute.components >= 1 && attribute.components <= 4,
            "Attribute %zu vector component count %u unsupported", i, attribute.components);
        checkMsg(
            attribute.binding < m_desc.bindings.size(),
            "Attribute %zu references unknown binding %u", i, attribute.binding);

        checkMsg(
            (attribute.offset + attribute.size()) <= m_desc.bindings[attribute.binding].stride,
            "Attribute %zu position exceeds binding stride (offset: %u, size: %u, stride: %u)",
            i, attribute.offset, attribute.size(), m_desc.bindings[attribute.binding].stride);

        for (size_t j = 0; j < i; j++) {
            const VertexAttribute &other = m_desc.attributes[j];

            checkMsg(
                other.semantic != attribute.semantic || other.index != attribute.index,
                "Attribute %zu is semantic duplicate of %u", i, j);
        }
    }
}

/** Initialize the vertex data object.
 * @param count         Total number of vertices.
 * @param layout        Vertex data layout.
 * @param buffers       Array of buffers for each binding in the layout. */
GPUVertexData::GPUVertexData(size_t count, GPUVertexDataLayout *layout, GPUBufferArray &&buffers) :
    m_count(count),
    m_layout(layout),
    m_buffers(std::move(buffers))
{
    check(count);

    size_t expectedSize = m_layout->desc().bindings.size();
    checkMsg(
        m_buffers.size() == expectedSize,
        "Buffer count mismatch (expected %zu, got %zu)", expectedSize, m_buffers.size());

    for (size_t i = 0; i < m_buffers.size(); i++) {
        check(m_buffers[i]);
        check(m_buffers[i]->type() == GPUBuffer::kVertexBuffer);
    }
}
