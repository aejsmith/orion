/**
 * @file
 * @copyright           2015 Alex Smith
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
 *                      kMaterialTexturesEnd). Also, be careful reordering,
 *                      slot numbers are currently hardcoded in shader
 *                      definitions.
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
    };
}
