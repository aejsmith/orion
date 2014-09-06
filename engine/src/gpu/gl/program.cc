/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL GPU program implementation.
 *
 * We use the GL separable shaders extension, as this design is more in line
 * with other APIs. Our GPUProgram implementation holds a separable program
 * object with a single shader stage attached. Our GPUPipeline implementation
 * holds a program pipeline objects to which the separable programs are
 * attached.
 */

#include "gl.h"
#include "program.h"

#include <fstream>
#include <memory>
#include <streambuf>

/** Load a GL program.
 * @param path		Path to program.
 * @param type		Type of program. */
GLProgram::GLProgram(const char *path, Type type) :
	GPUProgram(type)
{
	std::ifstream stream(path);
	if(stream.fail())
		orion_abort("GL: Program `%s' not found", path);

	std::string source = std::string(
		std::istreambuf_iterator<char>(stream),
		std::istreambuf_iterator<char>());

	/* Compile the shader. */
	const GLchar *buf = source.c_str();
	m_program = glCreateShaderProgramv(gl::convert_program_type(type), 1, &buf);
	if(!m_program)
		orion_abort("GL: Failed to create program object");

	/* Check whether it succeeded. glCreateShaderProgramv() appends the
	 * compiler log to the program info log if compilation fails, so this
	 * gets both compiler and linker errors. */
	GLint result;
	glGetProgramiv(m_program, GL_LINK_STATUS, &result);
	if(result != GL_TRUE) {
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &result);
		std::unique_ptr<char[]> log(new char[result]);
		glGetProgramInfoLog(m_program, result, &result, log.get());
		orion_log(LogLevel::kDebug, "Compiler log:\n%s", log.get());

		glDeleteProgram(m_program);
		orion_abort("GL: Failed to compile program `%s'", path);
	}
}

/** Destroy the program. */
GLProgram::~GLProgram() {
	glDeleteProgram(m_program);
}

/** Bind a uniform block in the program.
 * @param name		Name of uniform block.
 * @param index		Uniform buffer binding point index. */
void GLProgram::bind_uniforms(const char *name, unsigned index) {
	GLuint block_index = glGetUniformBlockIndex(m_program, name);
	if(block_index == GL_INVALID_INDEX)
		orion_abort("GL: Unknown uniform block '%s'", name);

	glUniformBlockBinding(m_program, block_index, index);
}

/** Bind a sampler in the program.
 * @param name		Name of sampler.
 * @param index		Texture unit index. */
void GLProgram::bind_texture(const char *name, unsigned index) {
	GLint location = glGetUniformLocation(m_program, name);
	if(location < 0)
		orion_abort("GL: Unknown sampler uniform name '%s'", name);

	glUniform1i(location, index);
}

/** Load a GPU program.
 * @param path		Path to the program source.
 * @param type		Type of the program.
 * @return		Pointer to created program. */
GPUProgramPtr GLGPUInterface::load_program(const char *path, GPUProgram::Type type) {
	GPUProgram *program = new GLProgram(path, type);
	return GPUProgramPtr(program);
}
