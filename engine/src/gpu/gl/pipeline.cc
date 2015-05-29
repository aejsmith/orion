/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               OpenGL pipeline implementation.
 */

#include "gl.h"
#include "pipeline.h"
#include "shader.h"

/** Construct the pipeline object.
 * @param shaders       Array of shaders for each stage. */
GLPipeline::GLPipeline(const GPUShaderArray &shaders) :
    GPUPipeline(shaders)
{
    glGenProgramPipelines(1, &m_pipeline);

    for (size_t i = 0; i < GPUShader::kNumShaderTypes; i++) {
        if (!m_shaders[i])
            continue;

        GLShader *shader = static_cast<GLShader *>(m_shaders[i].get());
        GLbitfield stage = gl::convertShaderTypeBitfield(shader->type());
        glUseProgramStages(m_pipeline, stage, shader->program());
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
GPUPipelinePtr GLGPUInterface::createPipeline(const GPUShaderArray &shaders) {
    return new GLPipeline(shaders);
}
