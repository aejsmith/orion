/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Global rendering resources.
 */

#include "gpu/gpu.h"

#include "render/resources.h"
#include "render/vertex.h"

/** Global rendering resources. */
EngineGlobal<RenderResources> g_renderResources;

/** Initialize the global rendering resources. */
RenderResources::RenderResources() {
    /* Create the simple vertex format. */
    VertexBufferLayoutArray buffers(1);
    buffers[0].stride = sizeof(SimpleVertex);
    VertexAttributeArray attributes(3);
    attributes[0].semantic = VertexAttribute::kPositionSemantic;
    attributes[0].index = 0;
    attributes[0].type = VertexAttribute::kFloatType;
    attributes[0].count = 3;
    attributes[0].buffer = 0;
    attributes[0].offset = offsetof(SimpleVertex, x);
    attributes[1].semantic = VertexAttribute::kNormalSemantic;
    attributes[1].index = 0;
    attributes[1].type = VertexAttribute::kFloatType;
    attributes[1].count = 3;
    attributes[1].buffer = 0;
    attributes[1].offset = offsetof(SimpleVertex, nx);
    attributes[2].semantic = VertexAttribute::kTexcoordSemantic;
    attributes[2].index = 0;
    attributes[2].type = VertexAttribute::kFloatType;
    attributes[2].count = 2;
    attributes[2].buffer = 0;
    attributes[2].offset = offsetof(SimpleVertex, u);
    m_simpleVertexFormat = g_gpu->createVertexFormat(buffers, attributes);
}

/** Destroy the rendering resources. */
RenderResources::~RenderResources() {}
