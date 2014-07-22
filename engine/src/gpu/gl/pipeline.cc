/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL pipeline implementation.
 */

#include "pipeline.h"
#include "program.h"

/** Construct the pipeline object. */
GLPipeline::GLPipeline() : m_pipeline(0) {}

/** Destroy the pipeline object. */
GLPipeline::~GLPipeline() {
	if(m_pipeline)
		glDeleteProgramPipelines(1, &m_pipeline);
}

/** Bind the pipeline for rendering. */
void GLPipeline::bind() {
	orion_assert(m_finalized);

	/* Note that monolithic program objects bound with glUseProgram take
	 * precedence over the bound pipeline object, so if glUseProgram is
	 * used anywhere, the program must be unbound when it is no longer
	 * needed for this to function correctly. */
	glBindProgramPipeline(m_pipeline);
}

/** Finalize the pipeline. */
void GLPipeline::_finalize() {
	glGenProgramPipelines(1, &m_pipeline);

	for(size_t i = 0; i < GPUProgram::kNumProgramTypes; i++) {
		if(!m_programs[i])
			continue;

		GLProgram *program = static_cast<GLProgram *>(m_programs[i].get());
		GLbitfield stage = gl::convert_program_type_bitfield(program->type());
		glUseProgramStages(m_pipeline, stage, program->program());
	}
}
