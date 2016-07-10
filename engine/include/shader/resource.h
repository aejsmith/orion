/*
 * Copyright (C) 2016 Alex Smith
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

#include "core/object.h"

/**
 * Standard resource set numbers.
 *
 * We group resources for shaders into sets, organised primarily by update
 * frequency. This maps well to APIs such as Vulkan and D3D12, and can be
 * emulated on other APIs as well. We define a standard set of resource set
 * numbers, listed below.
 */
namespace ResourceSets {
    enum ENUM() Value {
        /** Resources for the entity currently being rendered. */
        kEntityResources,
        /** Resources for the view the scene is being rendered from. */
        kViewResources,
        /** Resources for the light for the current pass. */
        kLightResources,
        /** Resources for the current material (contents defined by shader). */
        kMaterialResources,
    };
}

/** Resource slots for per-entity resources. */
namespace EntityResources {
    enum ENUM() Value {
        /** Uniforms for the entity. */
        kUniforms,
    };
}

/** Resource slots for per-view resources. */
namespace ViewResources {
    enum ENUM() Value {
        /** Uniforms for the view. */
        kUniforms,

        /** Deferred G-Buffer textures. */
        kDeferredBufferA,
        kDeferredBufferB,
        kDeferredBufferC,
        kDeferredBufferD,

        /** Depth buffer (should only be used by post-processing effects). */
        kDepthBuffer,

        /** Source texture for post-processing effects. */
        kSourceTexture,
    };
}

/** Resource slots for per-light resources. */
namespace LightResources {
    enum ENUM() Value {
        /** Uniforms for the light. */
        kUniforms,

        /** Shadow map. */
        kShadowMap,
    };
}
