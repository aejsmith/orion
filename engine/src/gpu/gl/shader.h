/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL shader implementation.
 */

#pragma once

#include "gl.h"

/** OpenGL GPU shader implementation. */
class GLShader : public GPUShader {
public:
	GLShader(const char *path, Type type);
	~GLShader();

	void bindUniforms(const char *name, unsigned index) override;
	void bindTexture(const char *name, unsigned index) override;

	/** Get the GL program object.
	 * @return		GL program object ID. */
	GLuint program() const { return m_program; }
private:
	GLuint m_program;		/**< Program object ID. */
};
