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
 * @brief               GL vertex data implementation.
 */

#pragma once

#include "buffer.h"

/** OpenGL vertex data implementation. */
class GLVertexData : public GPUVertexData {
public:
    GLVertexData(size_t count, GPUVertexDataLayout *layout, GPUBufferArray &&buffers);
    ~GLVertexData();

    void bind(GPUBuffer *indices);

    /** Get the VAO ID.
     * @return              VAO ID. */
    GLuint array() const { return m_array; }

    static bool mapAttribute(VertexAttribute::Semantic semantic, unsigned index, GLuint *gl);
private:
    GLuint m_array;                 /**< Vertex array object. */
    GPUBufferPtr m_boundIndices;    /**< Currently bound index buffer. */
};
