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
 * @brief               FXAA post-processing effect.
 */

#include "engine/asset_manager.h"

#include "render/effects/fxaa_effect.h"

/** Initialise the effect. */
FXAAEffect::FXAAEffect() {
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/fxaa_effect");
    m_material = new Material(shader);
}

/** Render the effect.
 * @param source        Reference to source image.
 * @param dest          Reference to destination image.
 * @return              Whether the effect was performed. */
bool FXAAEffect::render(GPUTexture *source, GPUTexture *dest) {
    /* Use bilinear filtering. */
    GPUSamplerStateDesc samplerDesc;
    samplerDesc.filterMode = SamplerFilterMode::kBilinear;
    samplerDesc.maxAnisotropy = 1;
    samplerDesc.addressU = samplerDesc.addressV = samplerDesc.addressW = SamplerAddressMode::kClamp;
    GPUSamplerStatePtr samplerState = g_gpuManager->createSamplerState(samplerDesc);

    blit(source, dest, m_material, 0, samplerState);
    return true;
}