/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               OpenGL GPU buffer implementation.
 */

#include "buffer.h"
#include "gl.h"

/** Initialize a new GL buffer.
 * @param type          Type of the buffer.
 * @param usage         Usage hint.
 * @param size          Buffer size. */
GLBuffer::GLBuffer(Type type, Usage usage, size_t size) :
    GPUBuffer(type, usage, size),
    m_glTarget(gl::convertBufferType(type)),
    m_glUsage(gl::convertBufferUsage(usage))
{
    glGenBuffers(1, &m_buffer);

    /* Create an initial data store. */
    g_opengl->state.bindBuffer(m_glTarget, m_buffer);
    glBufferData(m_glTarget, size, nullptr, m_glUsage);
}

/** Destroy the buffer. */
GLBuffer::~GLBuffer() {
    if (g_opengl->state.boundBuffers[m_glTarget] == m_buffer)
        g_opengl->state.boundBuffers[m_glTarget] = GL_NONE;

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

/** Write data to the buffer.
 * @param offset        Offset to write at.
 * @param size          Size of the data to write.
 * @param buf           Buffer containing data to write. */
void GLBuffer::writeImpl(size_t offset, size_t size, const void *buf) {
    g_opengl->state.bindBuffer(m_glTarget, m_buffer);

    if (offset == 0 && size == m_size) {
        glBufferData(m_glTarget, m_size, buf, m_glUsage);
    } else {
        glBufferSubData(m_glTarget, offset, size, buf);
    }
}

/** Map the buffer.
 * @param offset        Offset to map from.
 * @param size          Size of the range to map.
 * @param flags         Bitmask of mapping behaviour flags (see MapFlags).
 * @param access        Bitmask of access flags.
 * @return              Pointer to mapped buffer. */
void *GLBuffer::mapImpl(size_t offset, size_t size, uint32_t flags, uint32_t access) {
    uint32_t gl = 0;

    if (flags & kMapInvalidate)
        gl |= GL_MAP_INVALIDATE_RANGE_BIT;
    if (flags & kMapInvalidateBuffer)
        gl |= GL_MAP_INVALIDATE_BUFFER_BIT;

    if (access & kReadAccess)
        gl |= GL_MAP_READ_BIT;
    if (access & kWriteAccess)
        gl |= GL_MAP_WRITE_BIT;

    g_opengl->state.bindBuffer(m_glTarget, m_buffer);

    /* If we are invalidating, reallocate storage explicitly. OS X's GL
     * implementation appears to be too stupid to do this itself, doing
     * it explictly here knocks a huge chunk off the time it takes to do
     * a buffer map. */
    if (flags & kMapInvalidateBuffer)
        glBufferData(m_glTarget, m_size, nullptr, m_glUsage);

    return glMapBufferRange(m_glTarget, offset, size, gl);
}

/** Unmap the previous mapping created for the buffer with _map(). */
void GLBuffer::unmapImpl() {
    g_opengl->state.bindBuffer(m_glTarget, m_buffer);
    glUnmapBuffer(m_glTarget);
}

/** Create a GPU buffer.
 * @see                 GPUBuffer::GPUBuffer().
 * @return              Pointer to created vertex buffer. */
GPUBufferPtr GLGPUInterface::createBuffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) {
    return new GLBuffer(type, usage, size);
}
