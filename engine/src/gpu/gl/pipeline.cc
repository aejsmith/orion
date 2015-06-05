/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               OpenGL pipeline implementation.
 */

#include "gl.h"
#include "pipeline.h"
#include "program.h"

/** Construct the pipeline object.
 * @param desc          Parameters for the pipeline. */
GLPipeline::GLPipeline(const GPUPipelineDesc &desc) :
    GPUPipeline(desc)
{
    glGenProgramPipelines(1, &m_pipeline);

    for (size_t i = 0; i < ShaderStage::kNumStages; i++) {
        if (!m_programs[i])
            continue;

        GLProgram *program = static_cast<GLProgram *>(m_programs[i].get());
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
    /* Note that monolithic program objects bound with glUseProgram take
     * precedence over the bound pipeline object, so if glUseProgram is used
     * anywhere, the program must be unbound when it is no longer needed for
     * this to function correctly. */
    g_opengl->state.bindPipeline(m_pipeline);
}

/** Create a pipeline object.
 * @see             GPUPipeline::GPUPipeline().
 * @return          Pointer to created pipeline. */
GPUPipelinePtr GLGPUManager::createPipeline(const GPUPipelineDesc &desc) {
    return new GLPipeline(desc);
}
