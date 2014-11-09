/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Rendering pipeline object.
 */

#pragma once

#include "gpu/shader.h"

#include <array>

/**
 * Rendering pipeline.
 *
 * This class groups together a set of GPU shaders to use for each pipeline
 * stage. Once created, a pipeline is immutable. Creation is performed through
 * GPUInterface::createPipeline().
 *
 * Modern APIs (DX12, Metal, Mantle) have the concept of pipeline state objects
 * that bundle up shaders along with some bits of state like blend mode, output
 * buffer format and vertex format, the goal being to avoid draw-time shader
 * recompilation for different states. Storing output buffer/vertex formats here
 * would be quite awkward to manage and would add unnecessary overhead for APIs
 * that do not need this. However, APIs that do can fairly easily cache created
 * state objects for different formats in their implementation of this class.
 */
class GPUPipeline : public GPUResource {
public:
	/** @return		Array of shaders used by the pipeline. */
	const GPUShaderArray &shaders() const { return m_shaders; }
protected:
	explicit GPUPipeline(const GPUShaderArray &shaders);
protected:
	GPUShaderArray m_shaders;	/**< Array of shaders for each stage. */

	/* For the default implementation of createPipeline(). */
	friend class GPUInterface;
};

/** Type of a reference to GPUPipeline. */
typedef GPUResourcePtr<GPUPipeline> GPUPipelinePtr;
