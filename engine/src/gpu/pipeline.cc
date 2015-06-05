/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Rendering pipeline object.
 */

#include "gpu/pipeline.h"

/** Initialize the pipeline.
 * @param desc          Parameters for the pipeline. */
GPUPipeline::GPUPipeline(const GPUPipelineDesc &desc) :
    m_shaders(desc.shaders)
{
    checkMsg(
        m_shaders[GPUShader::kVertexShader] && m_shaders[GPUShader::kFragmentShader],
        "A pipeline requires at least a vertex and a fragment shader");
}
