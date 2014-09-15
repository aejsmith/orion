/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL GPU shader implementation.
 *
 * We use the GL separable shaders extension, as this design is more in line
 * with other APIs. Our GPUShader implementation holds a separable program
 * object with a single shader stage attached. Our GPUPipeline implementation
 * holds a program pipeline object to which the separable programs are
 * attached.
 */

#include "gl.h"
#include "shader.h"

#include <fstream>
#include <memory>
#include <streambuf>

/** Load a GL shader.
 * @param path		Path to shader.
 * @param type		Type of shader. */
GLShader::GLShader(const char *path, Type type) :
	GPUShader(type)
{
	std::ifstream stream(path);
	if(stream.fail())
		orionAbort("GL: Program `%s' not found", path);

	std::string source = std::string(
		std::istreambuf_iterator<char>(stream),
		std::istreambuf_iterator<char>());

	/* Compile the shader. */
	const GLchar *buf = source.c_str();
	m_program = glCreateShaderProgramv(gl::convertShaderType(type), 1, &buf);
	if(!m_program)
		orionAbort("GL: Failed to create program object");

	/* Check whether it succeeded. glCreateShaderProgramv() appends the
	 * compiler log to the program info log if compilation fails, so this
	 * gets both compiler and linker errors. */
	GLint result;
	glGetProgramiv(m_program, GL_LINK_STATUS, &result);
	if(result != GL_TRUE) {
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &result);
		std::unique_ptr<char[]> log(new char[result]);
		glGetProgramInfoLog(m_program, result, &result, log.get());
		orionLog(LogLevel::kDebug, "Compiler log:\n%s", log.get());

		glDeleteProgram(m_program);
		orionAbort("GL: Failed to compile program `%s'", path);
	}
}

/** Destroy the shader. */
GLShader::~GLShader() {
	glDeleteProgram(m_program);
}

/** Bind a uniform block in the shader.
 * @param name		Name of uniform block.
 * @param index		Uniform buffer binding point index. */
void GLShader::bindUniforms(const char *name, unsigned index) {
	GLuint blockIndex = glGetUniformBlockIndex(m_program, name);
	if(blockIndex == GL_INVALID_INDEX)
		orionAbort("GL: Unknown uniform block '%s'", name);

	glUniformBlockBinding(m_program, blockIndex, index);
}

/** Bind a sampler in the shader.
 * @param name		Name of sampler.
 * @param index		Texture unit index. */
void GLShader::bindTexture(const char *name, unsigned index) {
	GLint uniformLocation = glGetUniformLocation(m_program, name);
	if(uniformLocation < 0)
		orionAbort("GL: Unknown sampler uniform name '%s'", name);

	glProgramUniform1i(m_program, uniformLocation, index);
}

/** Load a GPU shader.
 * @param path		Path to the shader source.
 * @param type		Type of the shader.
 * @return		Pointer to created shader. */
GPUShaderPtr GLGPUInterface::loadShader(const char *path, GPUShader::Type type) {
	GPUShader *program = new GLShader(path, type);
	return GPUShaderPtr(program);
}
