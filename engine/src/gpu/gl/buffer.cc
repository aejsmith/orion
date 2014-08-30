/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL GPU buffer implementation.
 */

#include "buffer.h"
#include "gl.h"

/** Initialize a new GL buffer.
 * @param type		Type of the buffer.
 * @param usage		Usage hint.
 * @param size		Buffer size. */
GLBuffer::GLBuffer(Type type, Usage usage, size_t size) :
	GPUBuffer(type, usage, size),
	m_gl_target(gl::convert_buffer_type(type)),
	m_gl_usage(gl::convert_buffer_usage(usage))
{
	glGenBuffers(1, &m_buffer);

	/* Create an initial data store. */
	g_opengl->state.bind_buffer(m_gl_target, m_buffer);
	glBufferData(m_gl_target, size, nullptr, m_gl_usage);
}

/** Destroy the buffer. */
GLBuffer::~GLBuffer() {
	if(g_opengl->state.bound_buffers[m_gl_target] == m_buffer)
		g_opengl->state.bound_buffers[m_gl_target] = GL_NONE;

	glDeleteBuffers(1, &m_buffer);
}

/** Bind the buffer. */
void GLBuffer::bind() const {
	g_opengl->state.bind_buffer(m_gl_target, m_buffer);
}

/** Bind the buffer to an indexed target.
 * @param index		Index to bind to. */
void GLBuffer::bind_indexed(unsigned index) const {
	/* Quoting the GL spec: "Each target represents an indexed array of
	 * buffer binding points, as well as a single general binding point
	 * that can be used by other buffer manipulation functions". This means
	 * that the general binding point used by bind() is separate and
	 * unaffected by this function, and vice-versa. */
	g_opengl->state.bind_buffer_base(m_gl_target, index, m_buffer);
}

/** Write data to the buffer.
 * @param offset	Offset to write at.
 * @param size		Size of the data to write.
 * @param buf		Buffer containing data to write. */
void GLBuffer::write_impl(size_t offset, size_t size, const void *buf) {
	g_opengl->state.bind_buffer(m_gl_target, m_buffer);

	if(offset == 0 && size == m_size) {
		glBufferData(m_gl_target, m_size, buf, m_gl_usage);
	} else {
		glBufferSubData(m_gl_target, offset, size, buf);
	}
}

/** Map the buffer.
 * @param offset	Offset to map from.
 * @param size		Size of the range to map.
 * @param flags		Bitmask of mapping behaviour flags (see MapFlags).
 * @param access	Bitmask of access flags.
 * @return		Pointer to mapped buffer. */
void *GLBuffer::map_impl(size_t offset, size_t size, uint32_t flags, uint32_t access) {
	uint32_t gl = 0;

	if(flags & kMapInvalidate)
		gl |= GL_MAP_INVALIDATE_RANGE_BIT;
	if(flags & kMapInvalidateBuffer)
		gl |= GL_MAP_INVALIDATE_BUFFER_BIT;

	if(access & kReadAccess)
		gl |= GL_MAP_READ_BIT;
	if(access & kWriteAccess)
		gl |= GL_MAP_WRITE_BIT;

	g_opengl->state.bind_buffer(m_gl_target, m_buffer);

	/* If we are invalidating, reallocate storage explicitly. OS X's GL
	 * implementation appears to be too stupid to do this itself, doing
	 * it explictly here knocks a huge chunk off the time it takes to do
	 * a buffer map. */
	if(flags & kMapInvalidateBuffer)
		glBufferData(m_gl_target, m_size, nullptr, m_gl_usage);

	return glMapBufferRange(m_gl_target, offset, size, gl);
}

/** Unmap the previous mapping created for the buffer with _map(). */
void GLBuffer::unmap_impl() {
	g_opengl->state.bind_buffer(m_gl_target, m_buffer);
	glUnmapBuffer(m_gl_target);
}

/** Create a GPU buffer.
 * @see		GPUBuffer::GPUBuffer().
 * @return	Pointer to created vertex buffer. */
GPUBufferPtr GLGPUInterface::create_buffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) {
	GPUBuffer *buffer = new GLBuffer(type, usage, size);
	return GPUBufferPtr(buffer);
}
