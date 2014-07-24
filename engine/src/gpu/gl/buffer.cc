/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL GPU buffer implementation.
 */

#include "buffer.h"

/** Initialize a new GL buffer.
 * @param type		Type of the buffer.
 * @param usage		Usage hint.
 * @param size		Buffer size. */
GLBuffer::GLBuffer(Type type, Usage usage, size_t size) :
	GPUBuffer(type, usage, size)
{
	glGenBuffers(1, &m_buffer);

	/* Create an initial data store. */
	GLenum target = gl::convert_buffer_type(type);
	GLenum hint = gl::convert_buffer_usage(usage);
	glBindBuffer(target, m_buffer);
	glBufferData(target, size, nullptr, hint);
}

/** Destroy the buffer. */
GLBuffer::~GLBuffer() {
	glDeleteBuffers(1, &m_buffer);
}

/** Bind the buffer. */
void GLBuffer::bind() const {
	GLenum target = gl::convert_buffer_type(m_type);
	glBindBuffer(target, m_buffer);
}

/** Bind the buffer to an indexed target.
 * @param index		Index to bind to. */
void GLBuffer::bind_indexed(unsigned index) const {
	/* Quoting the GL spec: "Each target represents an indexed array of
	 * buffer binding points, as well as a single general binding point
	 * that can be used by other buffer manipulation functions". This means
	 * that the general binding point used by bind() is separate and
	 * unaffected by this function, and vice-versa. */
	GLenum target = gl::convert_buffer_type(m_type);
	glBindBufferBase(target, index, m_buffer);
}

/** Write data to the buffer.
 * @param offset	Offset to write at.
 * @param size		Size of the data to write.
 * @param buf		Buffer containing data to write. */
void GLBuffer::_write(size_t offset, size_t size, const void *buf) {
	GLenum target = gl::convert_buffer_type(m_type);
	glBindBuffer(target, m_buffer);

	if(offset == 0 && size == m_size) {
		GLenum hint = gl::convert_buffer_usage(m_usage);
		glBufferData(target, m_size, buf, hint);
	} else {
		glBufferSubData(target, offset, size, buf);
	}
}

/** Map the buffer.
 * @param offset	Offset to map from.
 * @param size		Size of the range to map.
 * @param flags		Bitmask of mapping behaviour flags (see MapFlags).
 * @param access	Bitmask of access flags.
 * @return		Pointer to mapped buffer. */
void *GLBuffer::_map(size_t offset, size_t size, uint32_t flags, uint32_t access) {
	uint32_t gl = 0;

	if(flags & kMapInvalidate)
		gl |= GL_MAP_INVALIDATE_RANGE_BIT;
	if(flags & kMapInvalidateBuffer)
		gl |= GL_MAP_INVALIDATE_BUFFER_BIT;

	if(access & kReadAccess)
		gl |= GL_MAP_READ_BIT;
	if(access & kWriteAccess)
		gl |= GL_MAP_WRITE_BIT;

	GLenum target = gl::convert_buffer_type(m_type);
	glBindBuffer(target, m_buffer);
	return glMapBufferRange(target, offset, size, gl);
}

/** Unmap the previous mapping created for the buffer with _map(). */
void GLBuffer::_unmap() {
	GLenum target = gl::convert_buffer_type(m_type);
	glBindBuffer(target, m_buffer);
	glUnmapBuffer(target);
}
