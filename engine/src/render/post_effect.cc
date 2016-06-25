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
 * @brief               Post-processing effect class.
 */

#include "core/serialiser.h"

#include "render/draw_list.h"
#include "render/post_effect.h"
#include "render/render_manager.h"

#include "shader/slots.h"

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
    /* Set up a sampler state. */
    GPUSamplerStatePtr defaultState;
    if (!samplerState) {
        GPUSamplerStateDesc samplerDesc;
        samplerDesc.filterMode = SamplerFilterMode::kNearest;
        samplerDesc.maxAnisotropy = 1;
        samplerDesc.addressU = samplerDesc.addressV = samplerDesc.addressW = SamplerAddressMode::kClamp;
        defaultState = g_gpuManager->createSamplerState(samplerDesc);
        samplerState = defaultState;
    }

    /* Bind source texture. */
    g_gpuManager->bindTexture(TextureSlots::kSourceTexture, source, samplerState);

    /* Set the render target to the destination. */
    GPURenderTargetDesc rtDesc;
    rtDesc.numColours = 1;
    rtDesc.colour[0].texture = dest;
    g_gpuManager->setRenderTarget(&rtDesc);

    /* Disable blending and depth testing/writes. TODO: Alpha blending should
     * come from Pass properties. */
    g_gpuManager->setDepthStencilState<ComparisonFunc::kAlways, false>();
    g_gpuManager->setBlendState<>();

    /* Set up a draw call. */
    DrawList drawList;
    Geometry geometry;
    g_renderManager->quadGeometry(geometry);
    if (pass >= 0) {
        check(static_cast<size_t>(pass) < material->shader()->numPasses(Pass::kBasicPass));

        const Pass *shaderPass = material->shader()->pass(Pass::kBasicPass, pass);
        drawList.addDrawCall(geometry, material, nullptr, shaderPass);
    } else {
        drawList.addDrawCalls(geometry, material, nullptr, Pass::kBasicPass);
    }

    /* Draw it. */
    drawList.draw();
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
 * This function renders all effects in the chain. It accepts the colour and
 * depth buffers from the scene renderer as input, and returns a pointer to the
 * final output texture from post-processing chain. Upon return, the texture
 * returned will be the current render target.
 *
 * @param colour        Colour buffer from scene renderer.
 * @param depth         Depth buffer from scene renderer.
 * @param size          Size of the viewport (needed because the renderer's
 *                      buffers are not always the same size as the output).
 *
 * @return              Final processed output texture.
 */
GPUTexture *PostEffectChain::render(GPUTexture *colour, GPUTexture *depth, const glm::ivec2 &size) const {
    if (m_effects.empty())
        return colour;

    /* Bind depth texture. */
    GPUSamplerStateDesc samplerDesc;
    samplerDesc.filterMode = SamplerFilterMode::kNearest;
    samplerDesc.maxAnisotropy = 1;
    samplerDesc.addressU = samplerDesc.addressV = samplerDesc.addressW = SamplerAddressMode::kClamp;
    GPUSamplerStatePtr sampler = g_gpuManager->createSamplerState(samplerDesc);
    g_gpuManager->bindTexture(TextureSlots::kDepthBuffer, depth, sampler);

    /* We bounce between up to 2 temporary render targets. These are allocated
     * below when needed. TODO: I'd like to reuse the scene colour buffer here
     * when possible, it's a bit awkward just because of the possibility of it
     * being a different size and it doesn't matter too much right now. */
    GPUTexture *dest = nullptr;
    GPUTexture *source = colour;

    for (PostEffect *effect : m_effects) {
        if (!dest || dest == colour) {
            GPUTextureDesc textureDesc;
            textureDesc.type = GPUTexture::kTexture2D;
            textureDesc.width = size.x;
            textureDesc.height = size.y;
            textureDesc.mips = 1;
            textureDesc.flags = GPUTexture::kRenderTarget;
            textureDesc.format = colour->format();

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
