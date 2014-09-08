/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Rendering pipeline object.
 */

#include "gpu/pipeline.h"

/** Initialize the pipeline. */
GPUPipeline::GPUPipeline() : m_finalized(false) {}

/** Set the program to use for a stage.
 * @param stage		Stage to set for.
 * @param program	Program to set. Type must match stage. */
void GPUPipeline::set_program(GPUProgram::Type stage, const GPUProgramPtr &program) {
	orion_assert(!m_finalized);
	orion_assert(program->type() == stage);

	m_programs[stage] = program;
}

/** Finalize the pipeline. */
void GPUPipeline::finalize() {
	m_finalized = true;

	orion_check(
		m_programs[GPUProgram::kVertexProgram] && m_programs[GPUProgram::kFragmentProgram],
		"A pipeline requires at least a vertex and a fragment program");

	finalize_impl();
}
