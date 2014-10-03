/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Global rendering resources.
 */

#include "gpu/gpu.h"

#include "render/resources.h"
#include "render/vertex.h"

/** Global rendering resources. */
EngineGlobal<RenderResources> g_renderResources;

/** Initialize the global rendering resources. */
RenderResources::RenderResources() {
	/* Create the simple vertex format. */
	m_simpleVertexFormat = g_gpu->createVertexFormat();
	m_simpleVertexFormat->addBuffer(0, sizeof(SimpleVertex));
	m_simpleVertexFormat->addAttribute(
		VertexAttribute::kPositionSemantic, 0,
		VertexAttribute::kFloatType, 3, 0, offsetof(SimpleVertex, x));
	m_simpleVertexFormat->addAttribute(
		VertexAttribute::kNormalSemantic, 0,
		VertexAttribute::kFloatType, 3, 0, offsetof(SimpleVertex, nx));
	m_simpleVertexFormat->addAttribute(
		VertexAttribute::kTexcoordSemantic, 0,
		VertexAttribute::kFloatType, 2, 0, offsetof(SimpleVertex, u));
	m_simpleVertexFormat->finalize();
}

/** Destroy the rendering resources. */
RenderResources::~RenderResources() {}

