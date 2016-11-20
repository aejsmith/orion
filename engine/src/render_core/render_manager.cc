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
 * @brief               Rendering resource manager.
 */

#include "engine/asset_manager.h"

#include "gpu/gpu_manager.h"

#include "render_core/render_manager.h"
#include "render_core/utility.h"
#include "render_core/vertex.h"

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
        m_resources.simpleVertexDataLayout = g_gpuManager->getVertexDataLayout(desc);
    }

    /* Create the standard resource set layouts. */
    {
        GPUResourceSetLayoutDesc desc;

        /* Entity resources. */
        desc.slots.resize(ResourceSlots::kNumEntityResources);
        desc.slots[ResourceSlots::kUniforms].type = GPUResourceType::kUniformBuffer;
        m_resources.entityResourceSetLayout = g_gpuManager->createResourceSetLayout(std::move(desc));

        /* View resources. */
        desc.slots.resize(ResourceSlots::kNumViewResources);
        desc.slots[ResourceSlots::kUniforms].type = GPUResourceType::kUniformBuffer;
        desc.slots[ResourceSlots::kDeferredBufferA].type = GPUResourceType::kTexture;
        desc.slots[ResourceSlots::kDeferredBufferB].type = GPUResourceType::kTexture;
        desc.slots[ResourceSlots::kDeferredBufferC].type = GPUResourceType::kTexture;
        desc.slots[ResourceSlots::kDeferredBufferD].type = GPUResourceType::kTexture;
        m_resources.viewResourceSetLayout = g_gpuManager->createResourceSetLayout(std::move(desc));

        /* Light resources. */
        desc.slots.resize(ResourceSlots::kNumLightResources);
        desc.slots[ResourceSlots::kUniforms].type = GPUResourceType::kUniformBuffer;
        desc.slots[ResourceSlots::kShadowMap].type = GPUResourceType::kTexture;
        m_resources.lightResourceSetLayout = g_gpuManager->createResourceSetLayout(std::move(desc));

        /* Post effect resources. */
        desc.slots.resize(ResourceSlots::kNumPostEffectResources);
        desc.slots[ResourceSlots::kDepthBuffer].type = GPUResourceType::kTexture;
        desc.slots[ResourceSlots::kSourceTexture].type = GPUResourceType::kTexture;
        m_resources.postEffectResourceSetLayout = g_gpuManager->createResourceSetLayout(std::move(desc));
    }

    /* Create the standard render passes. */
    {
        GPURenderPassDesc desc;

        /* Shadow map pass. */
        desc.depthStencilAttachment.format = kShadowMapFormat;
        desc.depthStencilAttachment.loadOp = GPURenderLoadOp::kClear;
        desc.depthStencilAttachment.stencilLoadOp = GPURenderLoadOp::kDontCare;
        m_resources.sceneShadowMapPass = g_gpuManager->createRenderPass(std::move(desc));

        /* Deferred G-Buffer pass. */
        desc.colourAttachments.resize(3);
        desc.colourAttachments[0].format = kDeferredBufferAFormat;
        desc.colourAttachments[0].loadOp = GPURenderLoadOp::kClear;
        desc.colourAttachments[1].format = kDeferredBufferBFormat;
        desc.colourAttachments[1].loadOp = GPURenderLoadOp::kClear;
        desc.colourAttachments[2].format = kDeferredBufferCFormat;
        desc.colourAttachments[2].loadOp = GPURenderLoadOp::kClear;
        desc.depthStencilAttachment = GPURenderAttachmentDesc();
        desc.depthStencilAttachment.format = kScreenDepthBufferFormat;
        desc.depthStencilAttachment.loadOp = GPURenderLoadOp::kClear;
        desc.depthStencilAttachment.stencilLoadOp = GPURenderLoadOp::kClear;
        m_resources.sceneGBufferPass = g_gpuManager->createRenderPass(std::move(desc));

        /* Deferred lighting pass. */
        desc.colourAttachments.resize(1);
        desc.colourAttachments[0].format = kScreenColourBufferFormat;
        desc.colourAttachments[0].loadOp = GPURenderLoadOp::kClear;
        desc.depthStencilAttachment.format = kScreenDepthBufferFormat;
        desc.depthStencilAttachment.loadOp = GPURenderLoadOp::kLoad;
        desc.depthStencilAttachment.stencilLoadOp = GPURenderLoadOp::kLoad;
        m_resources.sceneLightPass = g_gpuManager->createRenderPass(std::move(desc));

        /* Forward pass (after deferred rendering, no clear). */
        desc.colourAttachments.resize(1);
        desc.colourAttachments[0].format = kScreenColourBufferFormat;
        desc.colourAttachments[0].loadOp = GPURenderLoadOp::kLoad;
        desc.depthStencilAttachment.format = kScreenDepthBufferFormat;
        desc.depthStencilAttachment.loadOp = GPURenderLoadOp::kLoad;
        desc.depthStencilAttachment.stencilLoadOp = GPURenderLoadOp::kLoad;
        m_resources.sceneForwardPass = g_gpuManager->createRenderPass(std::move(desc));

        /* Forward pass (no deferred rendering done, must clear). */
        desc.colourAttachments.resize(1);
        desc.colourAttachments[0].format = kScreenColourBufferFormat;
        desc.colourAttachments[0].loadOp = GPURenderLoadOp::kClear;
        desc.depthStencilAttachment.format = kScreenDepthBufferFormat;
        desc.depthStencilAttachment.loadOp = GPURenderLoadOp::kClear;
        desc.depthStencilAttachment.stencilLoadOp = GPURenderLoadOp::kClear;
        m_resources.sceneForwardClearPass = g_gpuManager->createRenderPass(std::move(desc));

        /* Post effect blit pass. */
        desc.colourAttachments.resize(1);
        desc.colourAttachments[0].format = kScreenColourBufferFormat;
        desc.colourAttachments[0].loadOp = GPURenderLoadOp::kDontCare;
        desc.depthStencilAttachment = GPURenderAttachmentDesc();
        m_resources.postEffectBlitPass = g_gpuManager->createRenderPass(std::move(desc));
    }

    /* Create the utility geometry. */
    {
        RenderUtil::makeQuad(m_resources.quadVertexData);
        RenderUtil::makeSphere(24, 24, m_resources.sphereVertexData, m_resources.sphereIndexData);
        RenderUtil::makeCone(20, m_resources.coneVertexData, m_resources.coneIndexData);
    }

    /* Load the deferred light material. */
    //{
    //    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/deferred_light");
    //    m_resources.deferredLightMaterial = new Material(shader);
    //}
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
        auto desc = GPUTextureDesc().
            setType(GPUTexture::kTexture2D).
            setWidth(rt.screenBufferSize.x).
            setHeight(rt.screenBufferSize.y).
            setMips(1).
            setFlags(GPUTexture::kRenderTarget);

        desc.format = kScreenColourBufferFormat;
        rt.colourBuffer = g_gpuManager->createTexture(desc);
        desc.format = kScreenDepthBufferFormat;
        rt.depthBuffer = g_gpuManager->createTexture(desc);
    }

    /* Re-allocate G-Buffer textures if necessary. */
    if (path == RenderPath::kDeferred && (rt.deferredBufferSize.x < size.x || rt.deferredBufferSize.y < size.y)) {
        /* Free old buffers. */
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

        /* Allocate the buffers. See render/defs.h for layout information. */
        auto desc = GPUTextureDesc().
            setType(GPUTexture::kTexture2D).
            setWidth(rt.deferredBufferSize.x).
            setHeight(rt.deferredBufferSize.y).
            setMips(1).
            setFlags(GPUTexture::kRenderTarget);

        desc.format = kDeferredBufferAFormat;
        rt.deferredBufferA = g_gpuManager->createTexture(desc);
        desc.format = kDeferredBufferBFormat;
        rt.deferredBufferB = g_gpuManager->createTexture(desc);
        desc.format = kDeferredBufferCFormat;
        rt.deferredBufferC = g_gpuManager->createTexture(desc);
        desc.format = kDeferredBufferDFormat;
        rt.deferredBufferD = g_gpuManager->createTexture(desc);
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
