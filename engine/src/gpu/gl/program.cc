/*
 * Copyright (C) 2015-2016 Alex Smith
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
 * We use the GL separable shaders extension to allow us to easily mix shaders
 * without being subject to the usual rules for linking between stages. Our
 * GPUProgram implementation holds a separable program object with a single
 * shader stage attached. Our GPUPipeline implementation holds a program
 * pipeline object to which the separable programs are attached.
 */

#include <spirv_glsl.hpp>

#include "gl.h"
#include "program.h"
#include "resource.h"

#include "core/string.h"

/** Initialize the program.
 * @param stage         Stage that the program is for.
 * @param program       Linked program object.
 * @param resources     Resource binding information. */
GLProgram::GLProgram(unsigned stage, GLuint program, ResourceList &&resources) :
    GPUProgram(stage),
    m_program(program),
    m_resources(std::move(resources))
{}

/** Destroy the program. */
GLProgram::~GLProgram() {
    glDeleteProgram(m_program);
}

/** Update resource bindings in the program.
 * @param layouts       Layout set that the program is being used with. */
void GLProgram::setResourceLayout(const GPUResourceSetLayoutArray &layouts) {
    /* We've already validated the layout compatiblity with the shader when we
     * created the pipeline. No need to check again here. */
    for (Resource &resource : m_resources) {
        GLResourceSetLayout *layout = static_cast<GLResourceSetLayout *>(layouts[resource.set].get());
        unsigned binding = layout->mapSlot(resource.set, resource.slot);

        if (binding != resource.current) {
            switch (resource.type) {
                case GPUResourceType::kUniformBuffer:
                    glUniformBlockBinding(m_program, resource.location, binding);
                    break;
                case GPUResourceType::kTexture:
                    glProgramUniform1i(m_program, resource.location, binding);
                    break;
                default:
                    check(false);
            }

            resource.current = binding;
        }
    }
}

/** Get and fix up resources from a SPIR-V shader.
 * @param compiler      Cross compiler.
 * @return              Generated resource list. */
static GLProgram::ResourceList getResources(spirv_cross::CompilerGLSL &compiler) {
    GLProgram::ResourceList resources;

    spirv_cross::ShaderResources spvResources = compiler.get_shader_resources();

    auto getResource =
        [&] (const spirv_cross::Resource &spvResource, GPUResourceType type) {
            resources.emplace_back();
            GLProgram::Resource &resource = resources.back();
            resource.name = spvResource.name;
            resource.type = type;
            resource.set  = compiler.get_decoration(spvResource.id, spv::DecorationDescriptorSet);
            resource.slot = compiler.get_decoration(spvResource.id, spv::DecorationBinding);
            resource.current = -1u;

            compiler.unset_decoration(spvResource.id, spv::DecorationDescriptorSet);
            compiler.unset_decoration(spvResource.id, spv::DecorationBinding);
        };

    for (const auto &spvResource : spvResources.uniform_buffers)
        getResource(spvResource, GPUResourceType::kUniformBuffer);

    for (const auto &spvResource : spvResources.sampled_images)
        getResource(spvResource, GPUResourceType::kTexture);

    return resources;
}

/** Generate GLSL source from the SPIR-V.
 * @param compiler      Cross compiler.
 * @param stage         Stage that the program is for.
 * @return              Generated source string. */
static std::string generateSource(spirv_cross::CompilerGLSL &compiler, unsigned stage) {
    GLFeatures &features = g_opengl->features;

    spirv_cross::CompilerGLSL::Options options;
    options.version = (features.versionMajor * 100) + (features.versionMinor * 10);
    options.es = false;
    options.vulkan_semantics = false;
    options.vertex.fixup_clipspace = false;
    compiler.set_options(options);

    compiler.add_header_line("#extension GL_ARB_separate_shader_objects : enable");

    if (stage == ShaderStage::kVertex) {
        /* For some absurd reason SSO requires the gl_PerVertex block to be
         * redeclared. Do so here so we don't have to do it in every shader. */
        compiler.add_header_line("out gl_PerVertex { vec4 gl_Position; };\n");
    }

    return compiler.compile();
}

/** Create a GPU program from a SPIR-V binary.
 * @param stage         Stage that the program is for.
 * @param spirv         SPIR-V binary for the shader.
 * @return              Pointer to created shader on success, null on error. */
GPUProgramPtr GLGPUManager::createProgram(unsigned stage, const std::vector<uint32_t> &spirv) {
    spirv_cross::CompilerGLSL compiler(spirv);

    /* See resource.cc for a description of how we handle resource bindings.
     * Here we record the resource set binding information in the SPIR-V shader
     * and remove it before translating back to GLSL. */
    GLProgram::ResourceList resources = getResources(compiler);

    /* Translate the SPIR-V back to GLSL. Hopefully future GL versions will
     * gain support for consuming SPIR-V directly. We would still need to do
     * the resource remapping, though. */
    std::string source = generateSource(compiler, stage);

    /* Compile the shader. */
    GLuint shader = glCreateShader(GLUtil::convertShaderStage(stage));
    if (!shader) {
        logError("GL: Failed to create shader object");
        return nullptr;
    }

    const GLchar *string = source.c_str();
    glShaderSource(shader, 1, &string, nullptr);
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

    /* Get uniform locations for the resources from the linked program. These
     * may not be active if they are not used, so remove missing resources from
     * the list. */
    for (auto resource = resources.begin(); resource != resources.end(); ) {
        switch (resource->type) {
            case GPUResourceType::kUniformBuffer:
            {
                GLuint ret = glGetUniformBlockIndex(program, resource->name.c_str());
                if (ret == GL_INVALID_INDEX) {
                    resources.erase(resource++);
                    continue;
                }

                resource->location = ret;
                break;
            }

            case GPUResourceType::kTexture:
            {
                GLint ret = glGetUniformLocation(program, resource->name.c_str());
                if (ret < 0) {
                    resources.erase(resource++);
                    continue;
                }

                resource->location = ret;
                break;
            }

            default:
                check(false);
        }

        ++resource;
    }

    return new GLProgram(stage, program, std::move(resources));
}
