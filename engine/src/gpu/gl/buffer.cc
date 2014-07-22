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

/** Write data to the buffer.
 * @param buf		Buffer containing data to write.
 * @param size		Size of the data to write.
 * @param offset	Offset to write at. */
void GLBuffer::write(const void *buf, size_t size, size_t offset) {
	orion_check((offset + size) <= m_size, "Write outside buffer bounds");

	GLenum target = gl::convert_buffer_type(m_type);
	glBindBuffer(target, m_buffer);

	if(offset == 0 && size == m_size) {
		GLenum hint = gl::convert_buffer_usage(m_usage);
		glBufferData(target, m_size, buf, hint);
	} else {
		glBufferSubData(target, offset, size, buf);
	}
}

/** Bind the buffer. */
void GLBuffer::bind() const {
	GLenum target = gl::convert_buffer_type(m_type);
	glBindBuffer(target, m_buffer);
}
