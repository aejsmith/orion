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
 * @brief               Tonemapping post-processing effect.
 */

#include "engine/asset_manager.h"

#include "render/post_effects/tonemap_effect.h"

/** Initialise the effect. */
TonemapEffect::TonemapEffect() :
    exposure    (1.0f),
    whitePoint  (11.2f)
{
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/post_effects/tonemap_effect");
    m_material = new Material(shader);
}

/** Destroy the effect. */
TonemapEffect::~TonemapEffect() {}

/** @return             Expected input image type. */
RenderPipeline::ImageType TonemapEffect::inputImageType() const {
    return RenderPipeline::ImageType::kHDR;
}

/** @return             Output image type. */
RenderPipeline::ImageType TonemapEffect::outputImageType() const {
    return RenderPipeline::ImageType::kLinearLDR;
}

/** Render the effect.
 * @param source        Source texture.
 * @param target        Render target.
 * @param area          Area to render to on the target. */
void TonemapEffect::render(GPUTexture *source,
                           const GPURenderTargetDesc &target,
                           const IntRect &area) const
{
    m_material->setValue("exposure", this->exposure);
    m_material->setValue("whitePoint", this->whitePoint);

    blit(source, target, area, m_material);
}
