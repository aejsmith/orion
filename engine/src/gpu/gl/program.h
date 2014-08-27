/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL program implementation.
 */

#ifndef ORION_GPU_GL_PROGRAM_H
#define ORION_GPU_GL_PROGRAM_H

#include "defs.h"

/** OpenGL GPU program implementation. */
class GLProgram : public GPUProgram {
public:
	GLProgram(const char *path, Type type);
	~GLProgram();

	void bind_uniforms(const char *name, unsigned index) override;
	void bind_texture(const char *name, unsigned index) override;

	/** Get the GL program object.
	 * @return		GL program object ID. */
	GLuint program() const { return m_program; }
private:
	GLuint m_program;		/**< Program object ID. */
};

#endif /* ORION_GPU_GL_PROGRAM_H */
