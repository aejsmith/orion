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

#pragma once

#include "shader/material.h"

/**
 * Post-processing effect class.
 *
 * This class is the base of all post-processing effects which can be applied to
 * the image after rendering.
 */
class PostEffect {
public:
    /** Destroy the effect. */
    virtual ~PostEffect() {}

    /**
     * Render the effect.
     *
     * Given the source texture, renders to the destination texture with the
     * image effect applied. The source texture is the output of the previous
     * effect in the chain (or the renderer, if the effect is the first), and
     * the output will be either be used as the final image or as input into the
     * following effect in the chain.
     *
     * @param source        Source texture.
     * @param dest          Destination texture.
     *
     * @return              Whether the effect was performed.
     */
    virtual bool render(GPUTexture *source, GPUTexture *dest) = 0;
protected:
    /** Initialise the effect. */
    PostEffect() {}

    void blit(GPUTexture *source, GPUTexture *dest, Material *material, int pass = -1);
};

/**
 * Post-processing effect chain.
 *
 * This class maintains an ordered list of post-processing effects that will
 * be applied after rendering.
 */
class PostEffectChain {
public:
    PostEffectChain();
    ~PostEffectChain();

    void addEffect(PostEffect *effect);

    GPUTexture *render(GPUTexture *colour, GPUTexture *depth, const glm::ivec2 &size) const;
private:
    /** List of effects. */
    std::list<PostEffect *> m_effects;
};
