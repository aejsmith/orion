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

#include "engine/serialiser.h"

#include "render/post_effect.h"
#include "render/render_pipeline.h"

/** Global resources for all pipelines. */
static GlobalResource<RenderPipeline::BaseResources> g_renderPipelineResources;

/** Initialise global resources for all pipelines. */
RenderPipeline::BaseResources::BaseResources() {
    /* Create the standard render passes. */
    {
        GPURenderPassDesc desc;

        desc.colourAttachments.resize(1);
        desc.colourAttachments[0].format = kColourBufferFormat;
        desc.colourAttachments[0].loadOp = GPURenderLoadOp::kDontCare;
        desc.depthStencilAttachment = GPURenderAttachmentDesc();
        this->postEffectPass = g_gpuManager->createRenderPass(std::move(desc));
    }
}

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

/** Render all post-processing effects.
 * @param input         Input texture.
 * @return              Final processed output texture. */
RenderTargetPool::Handle RenderPipeline::renderPostEffects(const RenderTargetPool::Handle &input) const {
    if (m_postEffects.empty())
        return input;

    GPU_DEBUG_GROUP("Post Processing");

    /* We bounce between up to 2 temporary render targets. These are allocated
     * below when needed. */
    RenderTargetPool::Handle dest;
    RenderTargetPool::Handle source = input;

    for (PostEffect *effect : m_postEffects) {
        GPU_DEBUG_GROUP("%s", effect->metaClass().name());

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

        if (effect->render(source, dest)) {
            /* Rendered successfully, switch the output to be the source for the
             * next effect. */
            std::swap(dest, source);
        }
    }

    /* Swapped above, last output is currently in source. */
    return source;
}
