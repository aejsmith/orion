/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Global rendering resources.
 */

#pragma once

#include "gpu/vertex_format.h"

/** Global rendering resources. */
class RenderResources : Noncopyable {
public:
	RenderResources();
	~RenderResources();

	/** Get the GPU vertex format corresponding to SimpleVertex.
	 * @return		Simple vertex format. */
	GPUVertexFormat *simpleVertexFormat() const { return m_simpleVertexFormat; }
private:
	GPUVertexFormatPtr m_simpleVertexFormat;
};

extern EngineGlobal<RenderResources> g_renderResources;