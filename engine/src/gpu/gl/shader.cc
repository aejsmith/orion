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

/** Target GLSL version. */
static const char *kTargetGLSLVersion = "330 core";

/** Initialize the shader.
 * @param type		Type of shader.
 * @param program	Linked program object. */
GLShader::GLShader(Type type, GLuint program) :
	GPUShader(type),
	m_program(program)
{}

/** Destroy the shader. */
GLShader::~GLShader() {
	glDeleteProgram(m_program);
}

/** Query active uniform blocks in the program.
 * @param list		Resource list to fill in. */
void GLShader::queryUniformBlocks(ResourceList &list) {
	GLint numBlocks = 0;
	glGetProgramiv(m_program, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);

	for(GLint i = 0; i < numBlocks; i++) {
		GLint nameLen = 0;
		glGetActiveUniformBlockiv(m_program, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLen);

		char name[nameLen + 1];
		glGetActiveUniformBlockName(m_program, i, nameLen, &nameLen, &name[0]);
		name[nameLen] = 0;

		list.push_back({ std::string(name), static_cast<unsigned>(i) });
	}
}

/** Query active texture samplers in the program.
 * @param list		Resource list to fill in. */
void GLShader::querySamplers(ResourceList &list) {
	GLint numUniforms = 0;
	glGetProgramiv(m_program, GL_ACTIVE_UNIFORMS, &numUniforms);

	for(GLuint i = 0; i < static_cast<GLuint>(numUniforms); i++) {
		/* This range includes uniforms in a uniform block. Skip them,
		 * samplers cannot be specified in uniform blocks. */
		GLint blockIndex = 0;
		glGetActiveUniformsiv(m_program, 1, &i, GL_UNIFORM_BLOCK_INDEX, &blockIndex);
		if(blockIndex >= 0)
			continue;

		/* Query the type to check if it's a sampler. */
		GLint type = 0;
		glGetActiveUniformsiv(m_program, 1, &i, GL_UNIFORM_TYPE, &type);
		switch(type) {
		case GL_SAMPLER_1D:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_1D_SHADOW:
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_1D_ARRAY:
		case GL_SAMPLER_2D_ARRAY:
		case GL_SAMPLER_1D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_MULTISAMPLE:
		case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_SAMPLER_CUBE_SHADOW:
			break;
		default:
			/* TODO: other sampler types? */
			continue;
		}

		GLint nameLen;
		glGetActiveUniformsiv(m_program, 1, &i, GL_UNIFORM_NAME_LENGTH, &nameLen);

		char name[nameLen + 1];
		glGetActiveUniformName(m_program, i, nameLen, &nameLen, &name[0]);
		name[nameLen] = 0;

		list.push_back({ std::string(name), static_cast<unsigned>(i) });
	}
}

/** Bind a uniform block in the shader.
 * @param index		Index of uniform block.
 * @param slot		Uniform buffer slot. */
void GLShader::bindUniformBlock(unsigned index, unsigned slot) {
	glUniformBlockBinding(m_program, index, slot);
}

/** Bind a texture sampler in the shader.
 * @param index		Index of sampler.
 * @param slot		Texture slot. */
void GLShader::bindSampler(unsigned index, unsigned slot) {
	glProgramUniform1i(m_program, index, slot);
}

/** Compile a GPU shader.
 * @param type		Type of the shader.
 * @param source	Shader source string.
 * @return		Pointer to created shader. */
GPUShaderPtr GLGPUInterface::compileShader(GPUShader::Type type, const std::string &source) {
	/* Add a version string at the start, and enable SSO. */
	std::string preamble = util::format("#version %s\n", kTargetGLSLVersion);
	preamble += "#extension GL_ARB_separate_shader_objects : enable\n";

	if(type == GPUShader::kVertexShader) {
		/* For some absurd reason SSO requires the gl_PerVertex block to
		 * be redeclared. Do so here so we don't have to do it in every
		 * shader. */
		preamble += "out gl_PerVertex { vec4 gl_Position; };\n";

		/* Insert attribute semantic definitions. TODO: Share this
		 * information with GLVertexData::mapAttribute(). */
		preamble += "#define kPositionSemantic 0\n";
		preamble += "#define kNormalSemantic 2\n";
		preamble += "#define kTexcoordSemantic 4\n";
		preamble += "#define kDiffuseSemantic 14\n";
		preamble += "#define kSpecularSemantic 15\n";
	}

	/* Compile the shader. */
	const GLchar *strings[] = { preamble.c_str(), source.c_str() };
	GLuint program = glCreateShaderProgramv(
		gl::convertShaderType(type),
		util::arraySize(strings),
		strings);
	if(!program) {
		logError("GL: Failed to create program object");
		return nullptr;
	}

	/* Check whether it succeeded. glCreateShaderProgramv() appends the
	 * compiler log to the program info log if compilation fails, so this
	 * gets both compiler and linker errors. */
	GLint result;
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if(result != GL_TRUE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &result);
		std::unique_ptr<char[]> log(new char[result]);
		glGetProgramInfoLog(program, result, &result, log.get());
		glDeleteProgram(program);

		logError("GL: Failed to compile shader");
		logInfo("GL: Compiler log:\n%s", log.get());
		return nullptr;
	}

	return new GLShader(type, program);
}
