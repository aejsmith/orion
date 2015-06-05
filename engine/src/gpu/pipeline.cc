/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Rendering pipeline object.
 */

#include "gpu/pipeline.h"

/** Initialize the pipeline.
 * @param desc          Parameters for the pipeline. */
GPUPipeline::GPUPipeline(const GPUPipelineDesc &desc) :
    m_programs(desc.programs)
{
    checkMsg(
        m_programs[ShaderStage::kVertex] && m_programs[ShaderStage::kFragment],
        "A pipeline requires at least a vertex and a fragment program");
}
