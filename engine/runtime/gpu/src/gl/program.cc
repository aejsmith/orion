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
            resource.name    = spvResource.name;
            resource.type    = type;
            resource.set     = compiler.get_decoration(spvResource.id, spv::DecorationDescriptorSet);
            resource.slot    = compiler.get_decoration(spvResource.id, spv::DecorationBinding);
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
 * @param name          Name of the shader.
 * @return              Generated source string. */
static std::string generateSource(spirv_cross::CompilerGLSL &compiler, unsigned stage, const std::string &name) {
    GLFeatures &features = g_opengl->features;

    spirv_cross::CompilerGLSL::Options options;
    options.version          = (features.versionMajor * 100) + (features.versionMinor * 10);
    options.es               = false;
    options.vulkan_semantics = false;
    compiler.set_options(options);

    /* For consistency with Vulkan we have NDC Z in the range [0, 1], but
     * OpenGL uses [-1, 1]. Fix this up. */
    options.vertex.fixup_clipspace = true;

    compiler.require_extension("GL_ARB_separate_shader_objects");

    std::string source = compiler.compile();

    /* SPIRV-Cross' add_header_line() is not useful for anything other than
     * preprocessor directives, as it adds the contents before any #extension
     * directives. All #extension directives must be first in the source, and
     * Mesa's compiler enforces this. Roll our own version that adds after any
     * extensions. */
    size_t insertionPos = 0;
    while (source[insertionPos] == '#' || source[insertionPos] == '\n') {
        insertionPos = source.find('\n', insertionPos);
        if (insertionPos == std::string::npos) {
            insertionPos = source.size();
            break;
        } else {
            insertionPos++;
        }
    }

    auto addHeader =
        [&] (const std::string &str) {
            source.insert(insertionPos, str);
            insertionPos += str.length();
        };

    /* Add a comment giving the shader name so it is visible in apitrace etc. */
    addHeader("/* " + name + " */\n\n");

    if (stage == ShaderStage::kVertex) {
        /* For some absurd reason SSO requires the gl_PerVertex block to be
         * redeclared. Do so here so we don't have to do it in every shader. */
        addHeader("out gl_PerVertex { vec4 gl_Position; };\n\n");
    }

    return source;
}

/** Initialize the program.
 * @param desc          Descriptor for the program. */
GLProgram::GLProgram(GPUProgramDesc &&desc) :
    GPUProgram (desc.stage)
{
    spirv_cross::CompilerGLSL compiler(desc.spirv);

    /* See resource.cc for a description of how we handle resource bindings.
     * Here we record the resource set binding information in the SPIR-V shader
     * and remove it before translating back to GLSL. */
    m_resources = getResources(compiler);

    /* Translate the SPIR-V back to GLSL. Hopefully future GL versions will
     * gain support for consuming SPIR-V directly. We would still need to do
     * the resource remapping, though. */
    std::string source = generateSource(compiler, desc.stage, desc.name);

    /* Compile the shader. */
    GLuint shader = glCreateShader(GLUtil::convertShaderStage(desc.stage));
    if (!shader)
        fatal("Failed to create GL shader object");

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

        logInfo("GL: Compiler log:\n%s", log.get());
        fatal("Failed to compile GL shader");
    }

    m_program = glCreateProgram();
    if (!m_program)
        fatal("Failed to create GL program object");

    /* Mark it as separable and link it. */
    glProgramParameteri(m_program, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(m_program, shader);
    glLinkProgram(m_program);

    /* Keep around the shader object if enabled. This means that the shader
     * objects will show up in OpenGL Profiler and allow their source to be
     * examined easily. */
    #if !ORION_GL_KEEP_SHADER_OBJECTS
        glDetachShader(m_program, shader);
        glDeleteShader(shader);
    #endif

    /* Check whether the linking succeeded. */
    glGetProgramiv(m_program, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &result);
        std::unique_ptr<char[]> log(new char[result]);
        glGetProgramInfoLog(m_program, result, &result, log.get());
        glDeleteProgram(m_program);

        logInfo("GL: Linker log:\n%s", log.get());
        fatal("Failed to link GL program");
    }

    /* Get uniform locations for the resources from the linked program. These
     * may not be active if they are not used, so remove missing resources from
     * the list. */
    for (auto resource = m_resources.begin(); resource != m_resources.end(); ) {
        switch (resource->type) {
            case GPUResourceType::kUniformBuffer:
            {
                GLuint ret = glGetUniformBlockIndex(m_program, resource->name.c_str());
                if (ret == GL_INVALID_INDEX) {
                    m_resources.erase(resource++);
                    continue;
                }

                resource->location = ret;
                break;
            }

            case GPUResourceType::kTexture:
            {
                GLint ret = glGetUniformLocation(m_program, resource->name.c_str());
                if (ret < 0) {
                    m_resources.erase(resource++);
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
}

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

/** Create a GPU program from a SPIR-V binary.
 * @param desc          Descriptor for the program.
 * @return              Pointer to created shader on success. */
GPUProgramPtr GLGPUManager::createProgram(GPUProgramDesc &&desc) {
    return new GLProgram(std::move(desc));
}
