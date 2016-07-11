/*
 * Copyright (C) 2015 Alex Smith
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
 * @brief               Rendering resource manager.
 */

#include "engine/asset_manager.h"

#include "gpu/gpu_manager.h"

#include "render/render_manager.h"
#include "render/utility.h"
#include "render/vertex.h"

/** Default rendering parameters. */
static const uint16_t kDefaultShadowMapResolution = 512;

/** Manager of global renderer resources */
RenderManager *g_renderManager;

/** Initialise the rendering manager. */
RenderManager::RenderManager() :
    m_shadowMapResolution(kDefaultShadowMapResolution)
{}

/** Create rendering resources. */
void RenderManager::init() {
    /* Create the simple vertex data layout. */
    GPUVertexDataLayoutDesc vertexDesc(1, 4);
    vertexDesc.bindings[0].stride = sizeof(SimpleVertex);
    vertexDesc.attributes[0].semantic = VertexAttribute::kPositionSemantic;
    vertexDesc.attributes[0].index = 0;
    vertexDesc.attributes[0].type = VertexAttribute::kFloatType;
    vertexDesc.attributes[0].components = 3;
    vertexDesc.attributes[0].binding = 0;
    vertexDesc.attributes[0].offset = offsetof(SimpleVertex, x);
    vertexDesc.attributes[1].semantic = VertexAttribute::kNormalSemantic;
    vertexDesc.attributes[1].index = 0;
    vertexDesc.attributes[1].type = VertexAttribute::kFloatType;
    vertexDesc.attributes[1].components = 3;
    vertexDesc.attributes[1].binding = 0;
    vertexDesc.attributes[1].offset = offsetof(SimpleVertex, nx);
    vertexDesc.attributes[2].semantic = VertexAttribute::kTexcoordSemantic;
    vertexDesc.attributes[2].index = 0;
    vertexDesc.attributes[2].type = VertexAttribute::kFloatType;
    vertexDesc.attributes[2].components = 2;
    vertexDesc.attributes[2].binding = 0;
    vertexDesc.attributes[2].offset = offsetof(SimpleVertex, u);
    vertexDesc.attributes[3].semantic = VertexAttribute::kDiffuseSemantic;
    vertexDesc.attributes[3].index = 0;
    vertexDesc.attributes[3].type = VertexAttribute::kFloatType;
    vertexDesc.attributes[3].components = 4;
    vertexDesc.attributes[3].binding = 0;
    vertexDesc.attributes[3].offset = offsetof(SimpleVertex, r);
    m_simpleVertexDataLayout = g_gpuManager->createVertexDataLayout(std::move(vertexDesc));

    /* Create the utility geometry. */
    RenderUtil::makeQuad(m_quadVertexData);
    RenderUtil::makeSphere(24, 24, m_sphereVertexData, m_sphereIndexData);
    RenderUtil::makeCone(20, m_coneVertexData, m_coneIndexData);

    /* Load the deferred light material. */
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/deferred_light");
    m_deferredLightMaterial = new Material(shader);
}

/**
 * Ensure render targets are allocated and sufficiently sized.
 *
 * This function is called at the beginning of scene rendering to ensure that
 * all render targets required by the render path are allocated and of
 * sufficient size.
 *
 * @param path          Rendering path being used.
 * @param size          Required dimensions (in pixels).
 */
void RenderManager::allocRenderTargets(RenderPath path, glm::ivec2 size) {
    RenderTargets &rt = m_renderTargets;

    /* Allocate main offscreen rendering textures. */
    if (rt.screenBufferSize.x < size.x || rt.screenBufferSize.y < size.y) {
        rt.colourBuffer = nullptr;
        rt.depthBuffer = nullptr;

        /* Calculate new size. We dynamically resize the render targets to be
         * the maximum that they need to be to satisfy all render targets so
         * we don't constantly have to reallocate them. */
        rt.screenBufferSize.x = std::max(rt.screenBufferSize.x, size.x);
        rt.screenBufferSize.y = std::max(rt.screenBufferSize.y, size.y);
        logDebug(
            "Resizing screen buffers to %dx%d (for %dx%d)",
            rt.screenBufferSize.x, rt.screenBufferSize.y, size.x, size.y);

        /* Allocate the buffers. */
        GPUTextureDesc desc;
        desc.type = GPUTexture::kTexture2D;
        desc.width = rt.screenBufferSize.x;
        desc.height = rt.screenBufferSize.y;
        desc.mips = 1;
        desc.flags = GPUTexture::kRenderTarget;
        desc.format = PixelFormat::kR8G8B8A8;
        rt.colourBuffer = g_gpuManager->createTexture(desc);
        desc.format = PixelFormat::kDepth24Stencil8;
        rt.depthBuffer = g_gpuManager->createTexture(desc);
    }

    /* Re-allocate G-Buffer textures if necessary. */
    if (path == RenderPath::kDeferred && (rt.deferredBufferSize.x < size.x || rt.deferredBufferSize.y < size.y)) {
        rt.deferredBufferA = nullptr;
        rt.deferredBufferB = nullptr;
        rt.deferredBufferC = nullptr;
        rt.deferredBufferD = nullptr;

        /* Calculate new size. We dynamically resize the render targets to be
         * the maximum that they need to be to satisfy all render targets so
         * we don't constantly have to reallocate them. */
        rt.deferredBufferSize.x = std::max(rt.deferredBufferSize.x, size.x);
        rt.deferredBufferSize.y = std::max(rt.deferredBufferSize.y, size.y);
        logDebug(
            "Resizing deferred buffers to %dx%d (for %dx%d)",
            rt.deferredBufferSize.x, rt.deferredBufferSize.y, size.x, size.y);

        /*
         * Allocate the buffers. The buffer layout is as follows:
         *
         *     | Format      | R          | G          | B          | A
         *  ---|-------------|------------|------------|------------|------------
         *   A | R10G10B10A2 | Normal.x   | Normal.y   | Normal.z   | -
         *  ---|-------------|------------|------------|------------|------------
         *   B | R8G8B8A8    | Diffuse.r  | Diffuse.g  | Diffuse.b  | -
         *  ---|-------------|------------|------------|------------|------------
         *   C | R8G8B8A8    | Specular.r | Specular.g | Specular.b | 1/Shininess
         *  ---|-------------|------------|------------|------------|------------
         *   D | D24S8       | Depth      | -          | -          | -
         *
         * These are all unsigned normalized textures, therefore the normals are
         * scaled to fit into the [0, 1] range, and the shininess is stored as
         * its reciprocal. Position is reconstructed from the depth buffer.
         */
        GPUTextureDesc desc;
        desc.type = GPUTexture::kTexture2D;
        desc.width = rt.deferredBufferSize.x;
        desc.height = rt.deferredBufferSize.y;
        desc.mips = 1;
        desc.flags = GPUTexture::kRenderTarget;
        desc.format = PixelFormat::kR10G10B10A2;
        rt.deferredBufferA = g_gpuManager->createTexture(desc);
        desc.format = PixelFormat::kR8G8B8A8;
        rt.deferredBufferB = g_gpuManager->createTexture(desc);
        rt.deferredBufferC = g_gpuManager->createTexture(desc);
        desc.format = PixelFormat::kDepth24Stencil8;
        rt.deferredBufferD = g_gpuManager->createTexture(desc);

        // FIXME: separate bindTexture/Sampler, and do this only once.
        GPUSamplerStateDesc samplerDesc;
        samplerDesc.filterMode = SamplerFilterMode::kNearest;
        samplerDesc.maxAnisotropy = 1;
        samplerDesc.addressU = samplerDesc.addressV = samplerDesc.addressW = SamplerAddressMode::kClamp;
        GPUSamplerStatePtr sampler = g_gpuManager->getSamplerState(samplerDesc);

        /* Update deferred buffer texture bindings. */
        g_gpuManager->bindTexture(TextureSlots::kDeferredBufferA, rt.deferredBufferA, sampler);
        g_gpuManager->bindTexture(TextureSlots::kDeferredBufferB, rt.deferredBufferB, sampler);
        g_gpuManager->bindTexture(TextureSlots::kDeferredBufferC, rt.deferredBufferC, sampler);
        g_gpuManager->bindTexture(TextureSlots::kDeferredBufferD, rt.deferredBufferD, sampler);
    }

    /* Mark all temporary render targets as free. TODO: Free up targets that
     * remain unused for a long period. */
    for (auto &target : m_tempRenderTargets)
        target.second.allocated = false;
}

/**
 * Allocate from the temporary render target pool.
 *
 * Allocates a texture matching the given parameters from the temporary render
 * target pool. These are to be used for things which are only needed within a
 * single SceneRenderer pass, such as shadow maps. All targets allocated from
 * the pool are marked as free for re-use at the next call to
 * allocRenderTargets().
 *
 * @param desc          Texture descriptor.
 *
 * @return              Pointer to allocated render target (not reference
 *                      counted, the texture is guaranteed to exist until the
 *                      next call to allocRenderTargets()).
 */
GPUTexture *RenderManager::allocTempRenderTarget(const GPUTextureDesc &desc) {
    /* See if we have a matching target spare in the pool. */
    auto range = m_tempRenderTargets.equal_range(desc);
    for (auto it = range.first; it != range.second; ++it) {
        if (!it->second.allocated) {
            it->second.allocated = true;
            return it->second.texture;
        }
    }

    logDebug(
        "Allocating new %ux%ux%u temporary render target of type %d",
        desc.width, desc.height,
        (desc.type == GPUTexture::kTexture2DArray || desc.type == GPUTexture::kTexture3D) ? desc.depth : 0,
        desc.type);

    /* Nothing found, create a new texture. */
    TempRenderTarget target;
    target.texture = g_gpuManager->createTexture(desc);
    target.allocated = true;
    m_tempRenderTargets.insert(std::make_pair(desc, target));
    return target.texture;
}
