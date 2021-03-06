/*
 * Copyright (C) 2015-2017 Alex Smith
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

#pragma once

#include "engine/object.h"

#include "gpu/render_pass.h"
#include "gpu/state.h"

#include "render/render_pipeline.h"

class GPUTexture;
class Material;

/**
 * Post-processing effect class.
 *
 * This class is the base of all post-processing effects which can be applied to
 * the image after rendering.
 */
class PostEffect : public Object {
public:
    CLASS();

    /** Destroy the effect. */
    ~PostEffect() {}

    /**
     * Get the expected type of input image.
     *
     * Some post-processing effects have requirements on the type of image they
     * are given as input. For example, tonemapping expects a HDR image, and
     * gamma correction expects a linear image. This is currently just used to
     * generate a warning if the post-processing chain is set up incorrectly.
     * Default is to return kDontCare.
     *
     * @return              Expected input image type.
     */
    virtual RenderPipeline::ImageType inputImageType() const { return RenderPipeline::ImageType::kDontCare; }

    /**
     * Get the type of output image.
     *
     * Some post-processing effects output a specific type of image. For
     * example, tonemapping generates an LDR image, and gamma correction
     * generates a non-linear image. This is used to pick an appropriate format
     * for the output texture passed to effect. Default is to return kDontCare,
     * which uses the same type as the input.
     *
     * @return              Output image type.
     */
    virtual RenderPipeline::ImageType outputImageType() const { return RenderPipeline::ImageType::kDontCare; }

    /**
     * Render the effect.
     *
     * Given the source texture, renders to the destination target with the
     * effect applied. The source texture is the output of the previous effect
     * in the chain (or the renderer, if the effect is the first), and the
     * target will be either be used as the final image or as input into the
     * following effect in the chain.
     *
     * @param source        Source texture.
     * @param target        Render target.
     * @param area          Area to render to on the target.
     */
    virtual void render(GPUTexture *source, const GPURenderTargetDesc &target, const IntRect &area) const = 0;
protected:
    /** Initialise the effect. */
    PostEffect() {}

    void blit(GPUTexture *source,
              const GPURenderTargetDesc &target,
              const IntRect &area,
              Material *material,
              size_t passIndex = 0,
              GPUSamplerState *samplerState = nullptr) const;
};
