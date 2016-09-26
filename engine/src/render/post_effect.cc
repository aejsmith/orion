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
 * @brief               Post-processing effect class.
 *
 * TODO:
 *  - Could we just use materials instead of managing our own resource sets?
 *    Still need to handle immutability after submission.
 */

#include "core/serialiser.h"

#include "render/draw_list.h"
#include "render/post_effect.h"
#include "render/render_manager.h"

/**
 * Blit from a source to a destination texture using a material.
 *
 * Draws a full-screen quad onto the destination texture using the specified
 * material. The material's shader is expected to contain one or more Basic
 * passes. If a pass index is specified, only that specific pass will be
 * performed, otherwise all basic passes in the shader will be performed.
 *
 * @param source        Source texture.
 * @param dest          Destination texture.
 * @param material      Material to draw with.
 * @param pass          If >= 0, a specific pass index to draw.
 * @param samplerState  Sampler state for sampling the source texture.
 */
void PostEffect::blit(
    GPUTexture *source,
    GPUTexture *dest,
    Material *material,
    int pass,
    GPUSamplerState *samplerState)
{
    const RenderManager::RenderTargets &targets = g_renderManager->renderTargets();

    /* Create a resource set. This must be done every time as resource sets are
     * immutable between submission and the end of the frame, and we have
     * differing resources each time. TODO: Could keep just a few resource sets
     * for the buffers we bounce between. */
    GPUResourceSetPtr resources = g_gpuManager->createResourceSet(
        g_renderManager->resources().postEffectResourceSetLayout);

    GPUSamplerStatePtr defaultSampler = g_gpuManager->getSamplerState(GPUSamplerStateDesc());

    /* Bind resources. */
    resources->bindTexture(ResourceSlots::kDepthBuffer, targets.depthBuffer, defaultSampler);
    resources->bindTexture(
        ResourceSlots::kSourceTexture,
        source,
        (samplerState) ? samplerState : defaultSampler.get());
    g_gpuManager->bindResourceSet(ResourceSets::kPostEffectResources, resources);

    /* Begin a render pass. */
    GPURenderPassInstanceDesc passDesc(g_renderManager->resources().postEffectBlitPass);
    passDesc.targets.colour[0].texture = dest;
    passDesc.renderArea = IntRect(0, 0, dest->width(), dest->height());
    g_gpuManager->beginRenderPass(passDesc);

    /* Disable blending and depth testing/writes. TODO: Blending should come
     * from Pass properties. */
    g_gpuManager->setDepthStencilState<ComparisonFunc::kAlways, false>();
    g_gpuManager->setBlendState<>();

    /* Set up a draw call. */
    DrawList drawList;
    Geometry geometry;
    g_renderManager->resources().quadGeometry(geometry);
    if (pass >= 0) {
        check(static_cast<size_t>(pass) < material->shader()->numPasses(Pass::Type::kBasic));

        const Pass *shaderPass = material->shader()->pass(Pass::Type::kBasic, pass);
        drawList.addDrawCall(geometry, material, nullptr, shaderPass);
    } else {
        drawList.addDrawCalls(geometry, material, nullptr, Pass::Type::kBasic);
    }

    /* Draw it. */
    drawList.draw();

    g_gpuManager->endRenderPass();
}

/** Initialise the post-processing effect chain. */
PostEffectChain::PostEffectChain() {}

/** Destroy the post-processing effect chain. */
PostEffectChain::~PostEffectChain() {}

/** Serialise the chain.
 * @param serialiser    Serialiser to write to. */
void PostEffectChain::serialise(Serialiser &serialiser) const {
    serialiser.beginArray("effects");

    for (PostEffect *effect : m_effects)
        serialiser.push(effect);

    serialiser.endArray();
}

/** Deserialise the chain.
 * @param serialiser    Serialiser to read from. */
void PostEffectChain::deserialise(Serialiser &serialiser) {
    if (serialiser.beginArray("effects")) {
        ObjectPtr<PostEffect> effect;
        while (serialiser.pop(effect))
            m_effects.emplace_back(std::move(effect));

        serialiser.endArray();
    }
}

/** Add an effect to the end of the chain.
 * @param effect        Effect to add. */
void PostEffectChain::addEffect(ObjectPtr<PostEffect> effect) {
    m_effects.emplace_back(std::move(effect));
}

/**
 * Render all effects in the chain.
 *
 * This function renders all effects in the chain on the current colour/depth
 * render targets from RenderManager. Returns a pointer to the final output
 * texture from post-processing chain. Upon return, the texture returned will
 * be the current render target.
 *
 * @param size          Size of the viewport (needed because the renderer's
 *                      buffers are not always the same size as the output).
 *
 * @return              Final processed output texture.
 */
GPUTexture *PostEffectChain::render(const glm::ivec2 &size) const {
    const RenderManager::RenderTargets &targets = g_renderManager->renderTargets();

    if (m_effects.empty())
        return targets.colourBuffer;

    GPU_DEBUG_GROUP("Post Processing");

    /* We bounce between up to 2 temporary render targets. These are allocated
     * below when needed. TODO: I'd like to reuse the scene colour buffer here
     * when possible, it's a bit awkward just because of the possibility of it
     * being a different size and it doesn't matter too much right now. */
    GPUTexture *dest = nullptr;
    GPUTexture *source = targets.colourBuffer;

    for (PostEffect *effect : m_effects) {
        GPU_DEBUG_GROUP("%s", effect->metaClass().name());

        if (!dest || dest == targets.colourBuffer) {
            auto textureDesc = GPUTextureDesc().
                setType(GPUTexture::kTexture2D).
                setWidth(size.x).
                setHeight(size.y).
                setMips(1).
                setFlags(GPUTexture::kRenderTarget).
                setFormat(targets.colourBuffer->format());

            dest = g_renderManager->allocTempRenderTarget(textureDesc);
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
