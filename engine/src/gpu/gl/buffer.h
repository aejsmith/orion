/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL buffer implementation.
 */

#ifndef ORION_GPU_GL_BUFFER_H
#define ORION_GPU_GL_BUFFER_H

#include "gl.h"

/** OpenGL GPU buffer implementation. */
class GLBuffer : public GPUBuffer {
public:
	GLBuffer(Type type, Usage usage, size_t size);
	~GLBuffer();

	void bind() const;
	void bind_indexed(unsigned index) const;

	/** Get the buffer ID.
	 * @return		Buffer ID. */
	GLuint buffer() const { return m_buffer; }
protected:
	void write_impl(size_t offset, size_t size, const void *buf) override;
	void *map_impl(size_t offset, size_t size, uint32_t flags, uint32_t access) override;
	void unmap_impl() override;
private:
	GLuint m_buffer;		/**< Buffer object ID. */
	GLenum m_gl_target;		/**< GL target. */
	GLenum m_gl_usage;		/**< GL usage. */
};

#endif /* ORION_GPU_GL_BUFFER_H */
