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
 * allows us to use a single VAO with any shader.
 */

#include "gl.h"
#include "vertex_data.h"

/** Initialize the vertex data object.
 * @param desc          Descriptor for the vertex data object. */
GLVertexData::GLVertexData(GPUVertexDataDesc &&desc) :
    GPUVertexData(std::move(desc)),
    m_boundIndices(nullptr)
{
    glGenVertexArrays(1, &m_array);
    g_opengl->state.bindVertexArray(m_array);

    for (const VertexAttribute &attribute : m_layout->desc().attributes) {
        GLuint index = attribute.glslIndex();

        /* FIXME: Check if type is supported. */
        GLenum type = GLUtil::convertAttributeType(attribute.type);
        void *offset = reinterpret_cast<void *>(static_cast<uintptr_t>(attribute.offset));

        const VertexBinding &binding = m_layout->desc().bindings[attribute.binding];
        const GLBuffer *buffer = static_cast<GLBuffer *>(m_buffers[attribute.binding].get());
        buffer->bind();

        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, attribute.components, type, attribute.normalised, binding.stride, offset);
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

/** Create a vertex data object.
 * @param desc          Descriptor for the vertex data object.
 * @return              Pointer to created vertex data object. */
GPUVertexDataPtr GLGPUManager::createVertexData(GPUVertexDataDesc &&desc) {
    return new GLVertexData(std::move(desc));
}
