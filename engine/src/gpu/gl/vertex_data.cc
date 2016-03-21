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
 *
 * GL vertex array objects store the bindings of vertex attributes to source
 * buffers, attribute data format, and the element array buffer binding.
 * Except for the index buffer binding, this is the same as what GPUVertexData
 * holds, therefore we can use VAOs to store the entire GPUVertexData state.
 * Despite the index buffer not being held by GPUVertexData we can additionally
 * make use of the VAO to store it - we hold a pointer to the last buffer used
 * with the VAO, and if the one being used for rendering is the same then we
 * don't rebind it.
 *
 * So that we don't tie a VAO to a specific shader, we bind shader attributes
 * to attribute indices statically based on the semantic and index of the
 * attribute, rather than letting the linker assign attribute indices. This
 * allows us to use a single VAO with any shader. GL implementations must
 * support a minimum of 16 attribute indices, so we divide up this space
 * between the various attribute semantics.
 */

#include "gl.h"
#include "vertex_data.h"

/** Initialize the vertex data object.
 * @param count         Total number of vertices.
 * @param format        Vertex format.
 * @param buffers       Array of buffers. */
GLVertexData::GLVertexData(size_t count, GPUVertexFormat *format, GPUBufferArray &buffers) :
    GPUVertexData(count, format, buffers),
    m_boundIndices(nullptr)
{
    glGenVertexArrays(1, &m_array);
    g_opengl->state.bindVertexArray(m_array);

    for (const VertexAttribute &attribute : m_format->attributes()) {
        GLuint index;
        if (!mapAttribute(attribute.semantic, attribute.index, &index))
            fatal("GL: Cannot map attribute (semantic: %d, index: %u)", attribute.semantic, attribute.index);

        /* FIXME: Check if type is supported. */
        GLenum type = GLUtil::convertAttributeType(attribute.type);
        void *offset = reinterpret_cast<void *>(static_cast<uintptr_t>(attribute.offset));

        const VertexBufferLayout &layout = m_format->buffers()[attribute.buffer];
        const GLBuffer *buffer = static_cast<GLBuffer *>(m_buffers[attribute.buffer].get());
        buffer->bind();

        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, attribute.count, type, attribute.normalised, layout.stride, offset);
    }
}

/** Destroy the vertex data object. */
GLVertexData::~GLVertexData() {
    if (g_opengl->state.boundVertexArray == m_array)
        g_opengl->state.bindVertexArray(g_opengl->defaultVertexArray);

    glDeleteVertexArrays(1, &m_array);
}

/** Bind the VAO for rendering.
 * @param indices       Index buffer being used for rendering. */
void GLVertexData::bind(GPUBuffer *indices) {
    g_opengl->state.bindVertexArray(m_array);

    /* As described at the top of the file, the index buffer binding is
     * part of VAO state. If the index buffer being used for rendering is
     * the same as the previous one used with this vertex data, we can avoid
     * a call to glBindBuffer here.
     *
     * We call glBindBuffer directly here as we don't want the binding we
     * set here to affect the global GLState. Additionally, GLState has a
     * special case to switch back to the default VAO if changing the index
     * buffer binding. */
    if (unlikely(indices != m_boundIndices)) {
        if (indices) {
            GLBuffer *buffer = static_cast<GLBuffer *>(indices);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->buffer());
        } else {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        m_boundIndices = indices;
    }
}

/** Map an attribute semantic/index to a GL attribute index.
 * @param semantic      Attribute semantic.
 * @param index         Attribute index.
 * @param gl            Where to store GL attribute index.
 * @return              Whether mapped successfully. */
bool GLVertexData::mapAttribute(VertexAttribute::Semantic semantic, unsigned index, GLuint *gl) {
    /* TODO: Make use of all supported hardware attributes rather than the
     * minimum of 16. Also, this is a somewhat arbitrary division for now,
     * may need tweaking based on future requirements (e.g. probably don't
     * need multiple positions). */

    /* If changing this, make sure to update defines in shader.cc. */
    switch (semantic) {
        case VertexAttribute::kPositionSemantic:
            if (index >= 2)
                return false;

            *gl = 0 + index;
            return true;
        case VertexAttribute::kNormalSemantic:
            if (index >= 2)
                return false;

            *gl = 2 + index;
            return true;
        case VertexAttribute::kTexcoordSemantic:
            if (index >= 10)
                return false;

            *gl = 4 + index;
            return true;
        case VertexAttribute::kDiffuseSemantic:
            if (index >= 1)
                return false;

            *gl = 14;
            return true;
        case VertexAttribute::kSpecularSemantic:
            if (index >= 1)
                return false;

            *gl = 15;
            return true;
    }

    return false;
}

/** Create a vertex data object.
 * @see             GPUVertexData::GPUVertexData().
 * @return          Pointer to created vertex data object. */
GPUVertexDataPtr GLGPUManager::createVertexData(size_t count, GPUVertexFormat *format, GPUBufferArray &buffers) {
    return new GLVertexData(count, format, buffers);
}
