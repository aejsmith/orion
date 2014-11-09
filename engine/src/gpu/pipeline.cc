/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Rendering pipeline object.
 */

#include "gpu/pipeline.h"

/** Initialize the pipeline.
 * @param shaders	Array of shaders for each stage, indexed by stage. */
GPUPipeline::GPUPipeline(const GPUShaderArray &shaders) :
	m_shaders(shaders)
{
	checkMsg(
		m_shaders[GPUShader::kVertexShader] && m_shaders[GPUShader::kFragmentShader],
		"A pipeline requires at least a vertex and a fragment shader");
}
