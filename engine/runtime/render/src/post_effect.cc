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
 * @brief               Post-processing effect class.
 */

#include "gpu/gpu_manager.h"

#include "render/post_effect.h"
#include "render/render_pipeline.h"

#include "render_core/geometry.h"
#include "render_core/material.h"
#include "render_core/pass.h"
#include "render_core/render_resources.h"

/** PostEffect pass type. */
static const std::string kPostEffectPassType("PostEffect");
DEFINE_PASS_TYPE(kPostEffectPassType, {});

/**
 * Blit from a source to a destination texture using a material.
 *
 * Draws a full-screen quad onto the destination target using the specified
 * material. The material's shader is expected to contain a PostEffect pass, and
 * accept a "sourceTexture" parameter which will be set to the source texture.
 *
 * @param source        Source texture.
 * @param target        Render target.
 * @param area          Area to render to on the target.
 * @param material      Material to draw with.
 * @param passIndex     Pass index to draw.
 * @param samplerState  Sampler state for sampling the source texture.
 */
void PostEffect::blit(
    GPUTexture *source,
    const GPURenderTargetDesc &target,
    const IntRect &area,
    Material *material,
    size_t passIndex,
    GPUSamplerState *samplerState) const
{
    /* Bind the source texture. */
    GPUSamplerStatePtr defaultSampler = g_gpuManager->getSamplerState();
    material->setGPUTexture(
        "sourceTexture",
        source,
        (samplerState) ? samplerState : defaultSampler.get());

    /* Begin a render pass matching the target format. Load op is set to don't
     * care since it is expected we will overwrite its entire content. */
    GPUCommandList *cmdList = RenderPipeline::beginSimpleRenderPass(
        target,
        area,
        GPURenderLoadOp::kDontCare);

    /* Disable blending and depth testing/writes. TODO: Blending should come
     * from Pass properties. */
    cmdList->setBlendState();
    cmdList->setDepthStencilState(GPUDepthStencilStateDesc().
        setDepthFunc  (ComparisonFunc::kAlways).
        setDepthWrite (false));

    /* Set rendering state. */
    material->setDrawState(cmdList);
    const Pass *pass = material->shader()->getPass(kPostEffectPassType, passIndex);
    pass->setDrawState(cmdList, ShaderKeywordSet());

    /* Draw a full-screen quad. */
    Geometry geometry = g_renderResources->quadGeometry();
    cmdList->draw(geometry.primitiveType, geometry.vertices, geometry.indices);
    g_gpuManager->submitRenderPass(cmdList);
}
