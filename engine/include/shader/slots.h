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
 * @brief               Standard resource binding point definitions.
 */

#pragma once

#include "core/core.h"

/**
 * Standard uniform buffer binding points.
 *
 * We define a set of standard uniform buffer binding point indices so that the
 * renderer can bind them once and have them automatically available to GPU
 * shaders.
 */
namespace UniformSlots {
    enum {
        /** Uniforms for the entity currently being rendered. */
        kEntityUniforms,
        /** Uniforms for the view the scene is being rendered from. */
        kViewUniforms,
        /** Uniforms for the light for the current pass. */
        kLightUniforms,
        /** Uniforms for the current material. */
        kMaterialUniforms,
    };
}

/**
 * Standard texture binding points.
 *
 * Similarly to UniformSlots, we define a set of standard texture binding point
 * indices. A range is kept free for per-shader textures to be bound to, the
 * others are for standard textures provided by the renderer, e.g. G-Buffer
 * textures, and are bound automatically.
 *
 * @note                Keep shader-specific definitions first, the texture
 *                      array in Material is indexed by slot (size is
 *                      kMaterialTexturesEnd).
 */
namespace TextureSlots {
    enum {
        /** Shader-specific texture range. */
        kMaterialTexturesStart = 0,
        kMaterialTexturesEnd = 15,

        /** Deferred G-Buffer textures. */
        kDeferredBufferA = 16,
        kDeferredBufferB = 17,
        kDeferredBufferC = 18,
        kDeferredBufferD = 19,

        /** Shadow map. */
        kShadowMap = 20,

        /** Depth buffer (should only be used by post-processing effects). */
        kDepthBuffer = 21,

        /** Source texture for post-processing effects. */
        kSourceTexture = 22,
    };
}
