/*
 * Copyright (C) 2015-2016 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               Global rendering resources.
 */

#include "engine/asset_manager.h"

#include "gpu/gpu_manager.h"

#include "render_core/render_resources.h"
#include "render_core/utility.h"
#include "render_core/vertex.h"

/** Global rendering resources. */
GlobalResource<RenderResources> g_renderResources;

/** Initialise the rendering resource manager. */
RenderResources::RenderResources() {
    /* Create the simple vertex data layout. */
    {
        GPUVertexDataLayoutDesc desc(1, 4);
        desc.bindings[0].stride = sizeof(SimpleVertex);
        desc.attributes[0].semantic = VertexAttribute::kPositionSemantic;
        desc.attributes[0].index = 0;
        desc.attributes[0].type = VertexAttribute::kFloatType;
        desc.attributes[0].components = 3;
        desc.attributes[0].binding = 0;
        desc.attributes[0].offset = offsetof(SimpleVertex, x);
        desc.attributes[1].semantic = VertexAttribute::kNormalSemantic;
        desc.attributes[1].index = 0;
        desc.attributes[1].type = VertexAttribute::kFloatType;
        desc.attributes[1].components = 3;
        desc.attributes[1].binding = 0;
        desc.attributes[1].offset = offsetof(SimpleVertex, nx);
        desc.attributes[2].semantic = VertexAttribute::kTexcoordSemantic;
        desc.attributes[2].index = 0;
        desc.attributes[2].type = VertexAttribute::kFloatType;
        desc.attributes[2].components = 2;
        desc.attributes[2].binding = 0;
        desc.attributes[2].offset = offsetof(SimpleVertex, u);
        desc.attributes[3].semantic = VertexAttribute::kDiffuseSemantic;
        desc.attributes[3].index = 0;
        desc.attributes[3].type = VertexAttribute::kFloatType;
        desc.attributes[3].components = 4;
        desc.attributes[3].binding = 0;
        desc.attributes[3].offset = offsetof(SimpleVertex, r);
        m_simpleVertexDataLayout = g_gpuManager->getVertexDataLayout(desc);
    }

    /* Create the standard resource set layouts. */
    {
        GPUResourceSetLayoutDesc desc;

        /* Entity resources. */
        desc.slots.resize(ResourceSlots::kNumEntityResources);
        desc.slots[ResourceSlots::kUniforms].type = GPUResourceType::kUniformBuffer;
        m_entityResourceSetLayout = g_gpuManager->createResourceSetLayout(std::move(desc));

        /* View resources. */
        desc.slots.resize(ResourceSlots::kNumViewResources);
        desc.slots[ResourceSlots::kUniforms].type = GPUResourceType::kUniformBuffer;
        m_viewResourceSetLayout = g_gpuManager->createResourceSetLayout(std::move(desc));

        /* Light resources. */
        desc.slots.resize(ResourceSlots::kNumLightResources);
        desc.slots[ResourceSlots::kUniforms].type = GPUResourceType::kUniformBuffer;
        desc.slots[ResourceSlots::kShadowMap].type = GPUResourceType::kTexture;
        m_lightResourceSetLayout = g_gpuManager->createResourceSetLayout(std::move(desc));
    }

    /* Create the utility geometry. */
    {
        RenderUtil::makeQuad(m_quadVertexData);
        RenderUtil::makeSphere(24, 24, m_sphereVertexData, m_sphereIndexData);
        RenderUtil::makeCone(20, m_coneVertexData, m_coneIndexData);
    }
}

/** Destroy the rendering resource manager. */
RenderResources::~RenderResources() {}
