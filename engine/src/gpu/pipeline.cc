/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Rendering pipeline object.
 */

#include "gpu/pipeline.h"

/** Initialize the pipeline. */
GPUPipeline::GPUPipeline() : m_finalized(false) {}

/** Add a shader to the pipeline.
 * @param shader	Shader to add. Any existing shader in the same stage
 *			will be replaced. */
void GPUPipeline::addShader(const GPUShaderPtr &shader) {
	check(!m_finalized);
	m_shaders[shader->type()] = shader;
}

/** Finalize the pipeline. */
void GPUPipeline::finalize() {
	m_finalized = true;

	checkMsg(
		m_shaders[GPUShader::kVertexShader] && m_shaders[GPUShader::kFragmentShader],
		"A pipeline requires at least a vertex and a fragment shader");

	finalizeImpl();
}
