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
		if(g_opengl->state.bound_pipeline == m_pipeline)
			g_opengl->state.bound_pipeline = GL_NONE;

		glDeleteProgramPipelines(1, &m_pipeline);
	}
}

/** Bind the pipeline for rendering. */
void GLPipeline::bind() {
	orion_assert(m_finalized);

	/* Note that monolithic program objects bound with glUseProgram take
	 * precedence over the bound pipeline object, so if glUseProgram is
	 * used anywhere, the program must be unbound when it is no longer
	 * needed for this to function correctly. */
	g_opengl->state.bind_pipeline(m_pipeline);
}

/** Finalize the pipeline. */
void GLPipeline::finalize_impl() {
	glGenProgramPipelines(1, &m_pipeline);

	for(size_t i = 0; i < GPUProgram::kNumProgramTypes; i++) {
		if(!m_programs[i])
			continue;

		GLProgram *program = static_cast<GLProgram *>(m_programs[i].get());
		GLbitfield stage = gl::convert_program_type_bitfield(program->type());
		glUseProgramStages(m_pipeline, stage, program->program());
	}
}

/** Create a pipeline object.
 * @return		Pointer to created pipeline. */
GPUPipelinePtr GLGPUInterface::create_pipeline() {
	GPUPipeline *pipeline = new GLPipeline();
	return GPUPipelinePtr(pipeline);
}
