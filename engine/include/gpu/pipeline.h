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
 * stage. Before a pipeline object can be used for rendering it must be
 * finalized, after which it becomes immutable. A pipeline object has an API-
 * specific implementation, therefore instances must be created via
 * GPUInterface::createPipeline().
 *
 * Modern APIs (DX12, Metal, Mantle) have the concept of pipeline state object
 * that bundle up shaders along with some bits of state like blend mode, output
 * buffer format and vertex format, the goal being to avoid draw-time shader
 * recompilation for different states. Storing output buffer/vertex formats here
 * would be quite awkward to manage and would add unnecessary overhead for APIs
 * that do not need this. However, APIs that do can fairly easily cache created
 * state objects for different formats in their implementation of this class.
 */
class GPUPipeline : public GPUResource {
public:
	void addShader(const GPUShaderPtr &shader);
	void finalize();

	/** Get the shader for a stage.
	 * @param stage		Stage to get shader for.
	 * @return		Shader set for that stage. */
	GPUShaderPtr shader(GPUShader::Type stage) const { return m_shaders[stage]; }
protected:
	/** Type of the shader array. */
	typedef std::array<GPUShaderPtr, GPUShader::kNumShaderTypes> ShaderArray;
protected:
	GPUPipeline();

	/** Called when the object is being finalized. */
	virtual void finalizeImpl() {}
protected:
	ShaderArray m_shaders;		/**< Array of shaders for each stage. */
	bool m_finalized;		/**< Whether the state has been finalized. */

	/* For the default implementation of createPipeline(). */
	friend class GPUInterface;
};

/** Type of a pipeline object pointer. */
typedef GPUResourcePtr<GPUPipeline> GPUPipelinePtr;
