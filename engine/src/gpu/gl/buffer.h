/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL buffer implementation.
 */

#ifndef ORION_GPU_GL_BUFFER_H
#define ORION_GPU_GL_BUFFER_H

#include "defs.h"

/** OpenGL GPU buffer implementation. */
class GLBuffer : public GPUBuffer {
public:
	GLBuffer(Type type, Usage usage, size_t size);
	~GLBuffer();

	void write(const void *buf, size_t size, size_t offset);

	void bind() const;

	/** Get the buffer ID.
	 * @return		Buffer ID. */
	GLuint buffer() const { return m_buffer; }
private:
	GLuint m_buffer;		/**< Buffer object ID. */
};

#endif /* ORION_GPU_GL_BUFFER_H */
