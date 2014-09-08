/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL program implementation.
 */

#pragma once

#include "gl.h"

/** OpenGL GPU program implementation. */
class GLProgram : public GPUProgram {
public:
	GLProgram(const char *path, Type type);
	~GLProgram();

	void bindUniforms(const char *name, unsigned index) override;
	void bindTexture(const char *name, unsigned index) override;

	/** Get the GL program object.
	 * @return		GL program object ID. */
	GLuint program() const { return m_program; }
private:
	GLuint m_program;		/**< Program object ID. */
};
