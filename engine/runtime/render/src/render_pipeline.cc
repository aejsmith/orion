/*
 * Copyright (C) 2017 Alex Smith
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
 * @brief               Rendering pipeline class.
 */

#include "engine/debug_manager.h"
#include "engine/serialiser.h"
#include "engine/window.h"

#include "render/post_effect.h"
#include "render/render_pipeline.h"
#include "render/render_view.h"

/** Global resources for all pipelines. */
static GlobalResource<RenderPipeline::BaseResources> g_renderPipelineResources;

/** Construct the pipeline. */
RenderPipeline::RenderPipeline() {
    /* Ensure global resources are initialised. */
    g_renderPipelineResources.init();
}

/** Destroy the pipeline. */
RenderPipeline::~RenderPipeline() {}

/** Serialise the chain.
 * @param serialiser    Serialiser to write to. */
void RenderPipeline::serialise(Serialiser &serialiser) const {
    Object::serialise(serialiser);

    serialiser.beginArray("postEffects");

    for (PostEffect *effect : m_postEffects)
        serialiser.push(effect);

    serialiser.endArray();
}

/** Deserialise the chain.
 * @param serialiser    Serialiser to read from. */
void RenderPipeline::deserialise(Serialiser &serialiser) {
    Object::deserialise(serialiser);

    if (serialiser.beginArray("postEffects")) {
        ObjectPtr<PostEffect> effect;
        while (serialiser.pop(effect))
            m_postEffects.emplace_back(std::move(effect));

        serialiser.endArray();
    }
}

/** @return             Global resources for all pipelines. */
const RenderPipeline::BaseResources &RenderPipeline::resources() {
    return *g_renderPipelineResources;
}

/** Add a post-processing effect to the end of the list.
 * @param effect        Effect to add. */
void RenderPipeline::addPostEffect(ObjectPtr<PostEffect> effect) {
    m_postEffects.emplace_back(std::move(effect));
}

/**
 * Render all post-processing effects.
 *
 * Renders all post-processing effects (if any) and outputs the final image to
 * the render target.
 *
 * @param input         Input texture.
 */
void RenderPipeline::renderPostEffects(RenderContext &context, const RenderTargetPool::Handle &input) const {
    if (m_postEffects.empty()) {
        /* Just blit to the output. */
        GPUTextureImageRef dest;
        context.target().getTextureImageRef(dest);
        g_gpuManager->blit(
            GPUTextureImageRef(input),
            dest,
            glm::ivec2(0, 0),
            context.view().viewport().pos(),
            context.view().viewport().size());
        return;
    }

    GPU_DEBUG_GROUP("Post Processing");

    /* We bounce between up to 2 temporary render targets. These are allocated
     * below when needed. */
    RenderTargetPool::Handle dest;
    RenderTargetPool::Handle source = input;

    for (PostEffect *effect : m_postEffects) {
        GPU_DEBUG_GROUP("%s", effect->metaClass().name());

        const bool isLast = effect == m_postEffects.back();

        GPURenderTargetDesc targetDesc(1);
        IntRect targetArea;

        if (isLast) {
            /* The final effect outputs onto the real render target. */
            context.target().getRenderTargetDesc(targetDesc);
            targetArea = context.view().viewport();
        } else {
            if (!dest || dest == input) {
                auto textureDesc = GPUTextureDesc().
                    setType   (GPUTexture::kTexture2D).
                    setWidth  (input->width()).
                    setHeight (input->height()).
                    setMips   (1).
                    setFlags  (GPUTexture::kRenderTarget).
                    setFormat (input->format());

                dest = g_renderTargetPool->allocate(textureDesc);
            }

            targetDesc.colour[0].texture = dest;
            targetArea = IntRect(0, 0, input->width(), input->height());
        }

        if (effect->render(source, targetDesc, targetArea)) {
            /* Rendered successfully, switch the output to be the source for the
             * next effect. */
            std::swap(dest, source);
        }
    }
}

/** Render debug primitives.
 * @param context       Rendering context. */
void RenderPipeline::renderDebug(RenderContext &context) const {
    GPU_DEBUG_GROUP("Debug");

    GPURenderTargetDesc targetDesc;
    context.target().getRenderTargetDesc(targetDesc);

    GPUCommandList *cmdList = beginSimpleRenderPass(
        targetDesc,
        context.view().viewport(),
        GPURenderLoadOp::kLoad);

    /* Draw debug primitives onto the view. */
    g_debugManager->renderView(cmdList, context.view().getResources());

    g_gpuManager->submitRenderPass(cmdList);
}

/**
 * Begin a simple render pass.
 *
 * Begins a simple render pass with a single colour attachment. These render
 * passes are cached and will be reused.
 *
 * @param desc          Descriptor for the colour attachment.
 */
GPUCommandList* RenderPipeline::beginSimpleRenderPass(
    const GPURenderTargetDesc &target,
    const IntRect &area,
    GPURenderLoadOp loadOp)
{
    assert(target.colour.size() == 1);
    assert(!target.depthStencil);

    /* Get a render pass matching the target format.
     * FIXME: Can we kill this special casing for the main window and always
     * have a real texture object here? */
    GPURenderAttachmentDesc attachmentDesc;
    attachmentDesc.format = (target.isMainWindow())
                                ? g_mainWindow->format()
                                : target.colour[0].texture->format();
    attachmentDesc.loadOp = loadOp;

    auto &cache = g_renderPipelineResources->renderPasses;

    auto it = cache.find(attachmentDesc);
    if (it == cache.end()) {
        GPURenderPassDesc passDesc(1);
        passDesc.colourAttachments[0] = attachmentDesc;

        GPURenderPassPtr pass = g_gpuManager->createRenderPass(std::move(passDesc));
        auto ret = cache.emplace(attachmentDesc, std::move(pass));
        check(ret.second);
        it = ret.first;
    }

    GPURenderPassInstanceDesc passDesc(it->second);
    passDesc.targets    = target;
    passDesc.renderArea = area;

    return g_gpuManager->beginRenderPass(passDesc);
}
