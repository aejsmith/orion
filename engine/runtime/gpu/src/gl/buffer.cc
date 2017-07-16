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
 * @brief               OpenGL GPU buffer implementation.
 */

#include "buffer.h"
#include "gl.h"

/** Initialize a new GL buffer.
 * @param desc          Descriptor for the buffer. */
GLBuffer::GLBuffer(const GPUBufferDesc &desc) :
    GPUBuffer  (desc),
    m_glTarget (GLUtil::convertBufferType(m_type)),
    m_glUsage  (GLUtil::convertBufferUsage(m_usage))
{
    glGenBuffers(1, &m_buffer);

    /* Create an initial data store. */
    g_opengl->state.bindBuffer(m_glTarget, m_buffer);
    glBufferData(m_glTarget, m_size, nullptr, m_glUsage);
}

/** Destroy the buffer. */
GLBuffer::~GLBuffer() {
    g_opengl->state.invalidateBuffer(m_glTarget, m_buffer);
    glDeleteBuffers(1, &m_buffer);
}

/** Bind the buffer. */
void GLBuffer::bind() const {
    g_opengl->state.bindBuffer(m_glTarget, m_buffer);
}

/** Bind the buffer to an indexed target.
 * @param index         Index to bind to. */
void GLBuffer::bindIndexed(unsigned index) const {
    /* Quoting the GL spec: "Each target represents an indexed array of
     * buffer binding points, as well as a single general binding point
     * that can be used by other buffer manipulation functions". This means
     * that the general binding point used by bind() is separate and
     * unaffected by this function, and vice-versa. */
    g_opengl->state.bindBufferBase(m_glTarget, index, m_buffer);
}

/** Map the buffer.
 * @param offset        Offset to map from.
 * @param size          Size of the range to map.
 * @param flags         Bitmask of mapping behaviour flags (see MapFlags).
 * @param access        Access mode.
 * @return              Pointer to mapped buffer. */
void *GLBuffer::map(size_t offset, size_t size, uint32_t flags, uint32_t access) {
    uint32_t glFlags = 0;

    check(size);
    check((offset + size) <= m_size);

    check(access == kWriteAccess);
    glFlags |= GL_MAP_WRITE_BIT;

    if (flags & kMapInvalidateBuffer || (offset == 0 && size == m_size)) {
        glFlags |= GL_MAP_INVALIDATE_BUFFER_BIT;
    } else {
        glFlags |= GL_MAP_INVALIDATE_RANGE_BIT;
    }

    g_opengl->state.bindBuffer(m_glTarget, m_buffer);

    /* If we are invalidating, reallocate storage explicitly. OS X's GL
     * implementation appears to be too stupid to do this itself, doing it
     * explictly here knocks a huge chunk off the time it takes to do a buffer
     * map. */
    if (glFlags & GL_MAP_INVALIDATE_BUFFER_BIT)
        glBufferData(m_glTarget, m_size, nullptr, m_glUsage);

    return glMapBufferRange(m_glTarget, offset, size, glFlags);
}

/** Unmap the previous mapping created for the buffer with map(). */
void GLBuffer::unmap() {
    g_opengl->state.bindBuffer(m_glTarget, m_buffer);
    glUnmapBuffer(m_glTarget);
}

/** Write data to the buffer.
 * @param offset        Offset to write at.
 * @param size          Size of the data to write.
 * @param buf           Buffer containing data to write.
 * @param flags         Mapping flags to use. */
void GLBuffer::write(size_t offset, size_t size, const void *buf, uint32_t flags) {
    g_opengl->state.bindBuffer(m_glTarget, m_buffer);

    if (offset == 0 && size == m_size) {
        glBufferData(m_glTarget, m_size, buf, m_glUsage);
    } else {
        if (flags & kMapInvalidateBuffer)
            glBufferData(m_glTarget, m_size, nullptr, m_glUsage);

        glBufferSubData(m_glTarget, offset, size, buf);
    }
}

/** Create a GPU buffer.
 * @param desc          Descriptor for the buffer..
 * @return              Pointer to created vertex buffer. */
GPUBufferPtr GLGPUManager::createBuffer(const GPUBufferDesc &desc) {
    return new GLBuffer(desc);
}
