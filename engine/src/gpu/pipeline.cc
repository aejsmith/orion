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
void GPUPipeline::setProgram(GPUProgram::Type stage, const GPUProgramPtr &program) {
	orionAssert(!m_finalized);
	orionAssert(program->type() == stage);

	m_programs[stage] = program;
}

/** Finalize the pipeline. */
void GPUPipeline::finalize() {
	m_finalized = true;

	orionCheck(
		m_programs[GPUProgram::kVertexProgram] && m_programs[GPUProgram::kFragmentProgram],
		"A pipeline requires at least a vertex and a fragment program");

	finalizeImpl();
}
