/*
 * Copyright (C) 2015 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               OpenGL GPU program implementation.
 *
 * We use the GL separable shaders extension, as this design is more in line
 * with other APIs. Our GPUProgram implementation holds a separable program
 * object with a single shader stage attached. Our GPUPipeline implementation
 * holds a program pipeline object to which the separable programs are
 * attached.
 */

#include "gl.h"
#include "program.h"

#include "core/string.h"

/** Target GLSL version. */
static const char *kTargetGLSLVersion = "330 core";

/** Initialize the program.
 * @param stage         Stage that the program is for.
 * @param program       Linked program object. */
GLProgram::GLProgram(unsigned stage, GLuint program) :
    GPUProgram(stage),
    m_program(program)
{}

/** Destroy the program. */
GLProgram::~GLProgram() {
    glDeleteProgram(m_program);
}

/** Query active uniform blocks in the program.
 * @param list          Resource list to fill in. */
void GLProgram::queryUniformBlocks(ResourceList &list) {
    GLint numBlocks = 0;
    glGetProgramiv(m_program, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);

    for (GLint i = 0; i < numBlocks; i++) {
        GLint nameLen = 0;
        glGetActiveUniformBlockiv(m_program, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLen);

        char name[nameLen + 1];
        glGetActiveUniformBlockName(m_program, i, nameLen, &nameLen, &name[0]);
        name[nameLen] = 0;

        list.push_back({ std::string(name), static_cast<unsigned>(i) });
    }
}

/** Query active texture samplers in the program.
 * @param list          Resource list to fill in. */
void GLProgram::querySamplers(ResourceList &list) {
    GLint numUniforms = 0;
    glGetProgramiv(m_program, GL_ACTIVE_UNIFORMS, &numUniforms);

    for (GLuint i = 0; i < static_cast<GLuint>(numUniforms); i++) {
        /* This range includes uniforms in a uniform block. Skip them, samplers
         * cannot be specified in uniform blocks. */
        GLint blockIndex = 0;
        glGetActiveUniformsiv(m_program, 1, &i, GL_UNIFORM_BLOCK_INDEX, &blockIndex);
        if (blockIndex >= 0)
            continue;

        /* Query the type to check if it's a sampler. */
        GLint type = 0;
        glGetActiveUniformsiv(m_program, 1, &i, GL_UNIFORM_TYPE, &type);
        switch (type) {
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

/** Bind a uniform block in the program.
 * @param index         Index of uniform block.
 * @param slot          Uniform buffer slot. */
void GLProgram::bindUniformBlock(unsigned index, unsigned slot) {
    glUniformBlockBinding(m_program, index, slot);
}

/** Bind a texture sampler in the program.
 * @param index         Index of sampler.
 * @param slot          Texture slot. */
void GLProgram::bindSampler(unsigned index, unsigned slot) {
    glProgramUniform1i(m_program, index, slot);
}

/** Compile a GPU program.
 * @param stage         Stage that the program is for.
 * @param source        Shader source string.
 * @return              Pointer to created shader. */
GPUProgramPtr GLGPUManager::compileProgram(unsigned stage, const std::string &source) {
    /* Add a version string at the start, and enable SSO. */
    std::string preamble = String::format("#version %s\n", kTargetGLSLVersion);
    preamble += "#extension GL_ARB_separate_shader_objects : enable\n";

    if (stage == ShaderStage::kVertex) {
        /* For some absurd reason SSO requires the gl_PerVertex block to be
         * redeclared. Do so here so we don't have to do it in every shader. */
        preamble += "out gl_PerVertex { vec4 gl_Position; };\n";

        /* Insert attribute semantic definitions. TODO: Share this information
         * with GLVertexData::mapAttribute(). */
        preamble += "#define kPositionSemantic 0\n";
        preamble += "#define kNormalSemantic 2\n";
        preamble += "#define kTexcoordSemantic 4\n";
        preamble += "#define kDiffuseSemantic 14\n";
        preamble += "#define kSpecularSemantic 15\n";
    }

    /* Compile the shader. */
    GLuint shader = glCreateShader(GLUtil::convertShaderStage(stage));
    if (!shader) {
        logError("GL: Failed to create shader object");
        return nullptr;
    }

    const GLchar *strings[] = { preamble.c_str(), source.c_str() };
    glShaderSource(shader, arraySize(strings), strings, NULL);
    glCompileShader(shader);

    /* Check whether the compilation succeeded. */
    GLint result = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result != GL_TRUE) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &result);
        std::unique_ptr<char[]> log(new char[result]);
        glGetShaderInfoLog(shader, result, &result, log.get());
        glDeleteShader(shader);

        logError("GL: Failed to compile shader");
        logInfo("GL: Compiler log:\n%s", log.get());
        return nullptr;
    }

    GLuint program = glCreateProgram();
    if (!program) {
        logError("GL: Failed to create program object");
        return nullptr;
    }

    /* Mark it as separable and link it. */
    glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(program, shader);
    glLinkProgram(program);

    /* Keep around the shader object if enabled. This means that the shader
     * objects will show up in OpenGL Profiler and allow their source to be
     * examined easily. */
    #if !ORION_GL_KEEP_SHADER_OBJECTS
        glDetachShader(program, shader);
        glDeleteShader(shader);
    #endif

    /* Check whether the linking succeeded. */
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &result);
        std::unique_ptr<char[]> log(new char[result]);
        glGetProgramInfoLog(program, result, &result, log.get());
        glDeleteProgram(program);

        logError("GL: Failed to link program");
        logInfo("GL: Linker log:\n%s", log.get());
        return nullptr;
    }

    return new GLProgram(stage, program);
}
