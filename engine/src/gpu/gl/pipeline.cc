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
 * @brief               OpenGL pipeline implementation.
 */

#include "gl.h"
#include "pipeline.h"
#include "program.h"

/** Construct the pipeline object.
 * @param desc          Parameters for the pipeline. */
GLPipeline::GLPipeline(GPUPipelineDesc &&desc) :
    GPUPipeline(std::move(desc))
{
    glGenProgramPipelines(1, &m_pipeline);

    for (size_t i = 0; i < ShaderStage::kNumStages; i++) {
        if (!m_programs[i])
            continue;

        GLProgram *program = static_cast<GLProgram *>(m_programs[i].get());

        /* Check whether this program is compatible with the layout. */
        for (const GLProgram::Resource &resource : program->resources()) {
            checkMsg(
                resource.set < m_resourceLayout.size() && m_resourceLayout[resource.set],
                "Shader resource '%s' wants set %u which is not in layout",
                resource.name.c_str(), resource.set);

            const GPUResourceSetLayoutDesc &desc = m_resourceLayout[resource.set]->desc();

            checkMsg(
                resource.slot < desc.slots.size() && desc.slots[resource.slot].type != GPUResourceType::kNone,
                "Shader resource '%s' wants set %u slot %u which is not in layout",
                resource.name.c_str(), resource.set, resource.slot);

            checkMsg(
                resource.type == desc.slots[resource.slot].type,
                "Shader resource '%s' (set %u slot %u) has type mismatch with layout (want %d, have %d)",
                resource.name.c_str(), resource.set, resource.slot,
                resource.type, desc.slots[resource.slot].type);
        }

        /* Bind to our pipeline. */
        GLbitfield stage = GLUtil::convertShaderStageBitfield(program->stage());
        glUseProgramStages(m_pipeline, stage, program->program());
    }
}

/** Destroy the pipeline object. */
GLPipeline::~GLPipeline() {
    g_opengl->state.invalidatePipeline(m_pipeline);
    glDeleteProgramPipelines(1, &m_pipeline);
}

/** Bind the pipeline for rendering. */
void GLPipeline::bind() {
    /* Update the resource bindings in the programs. */
    for (size_t i = 0; i < ShaderStage::kNumStages; i++) {
        if (m_programs[i]) {
            GLProgram *program = static_cast<GLProgram *>(m_programs[i].get());
            program->setResourceLayout(m_resourceLayout);
        }
    }

    /* Note that monolithic program objects bound with glUseProgram take
     * precedence over the bound pipeline object, so if glUseProgram is used
     * anywhere, the program must be unbound when it is no longer needed for
     * this to function correctly. */
    g_opengl->state.bindPipeline(m_pipeline);
}

/** Create a pipeline object.
 * @see             GPUPipeline::GPUPipeline().
 * @return          Pointer to created pipeline. */
GPUPipelinePtr GLGPUManager::createPipeline(GPUPipelineDesc &&desc) {
    return new GLPipeline(std::move(desc));
}
