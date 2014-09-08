/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL pipeline implementation.
 */

#include "gl.h"
#include "pipeline.h"
#include "program.h"

/** Construct the pipeline object. */
GLPipeline::GLPipeline() : m_pipeline(GL_NONE) {}

/** Destroy the pipeline object. */
GLPipeline::~GLPipeline() {
	if(m_pipeline != GL_NONE) {
		if(g_opengl->state.boundPipeline == m_pipeline)
			g_opengl->state.boundPipeline = GL_NONE;

		glDeleteProgramPipelines(1, &m_pipeline);
	}
}

/** Bind the pipeline for rendering. */
void GLPipeline::bind() {
	orionAssert(m_finalized);

	/* Note that monolithic program objects bound with glUseProgram take
	 * precedence over the bound pipeline object, so if glUseProgram is
	 * used anywhere, the program must be unbound when it is no longer
	 * needed for this to function correctly. */
	g_opengl->state.bindPipeline(m_pipeline);
}

/** Finalize the pipeline. */
void GLPipeline::finalizeImpl() {
	glGenProgramPipelines(1, &m_pipeline);

	for(size_t i = 0; i < GPUProgram::kNumProgramTypes; i++) {
		if(!m_programs[i])
			continue;

		GLProgram *program = static_cast<GLProgram *>(m_programs[i].get());
		GLbitfield stage = gl::convertProgramTypeBitfield(program->type());
		glUseProgramStages(m_pipeline, stage, program->program());
	}
}

/** Create a pipeline object.
 * @return		Pointer to created pipeline. */
GPUPipelinePtr GLGPUInterface::createPipeline() {
	GPUPipeline *pipeline = new GLPipeline();
	return GPUPipelinePtr(pipeline);
}
