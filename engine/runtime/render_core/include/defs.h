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
 * @brief               Core rendering definitions.
 */

#pragma once

#include "engine/object.h"

#include <set>

/** Set of shader variation keywords. */
using ShaderKeywordSet = std::set<std::string>;

/**
 * Standard resource set numbers.
 *
 * We group resources for shaders into sets, organised primarily by update
 * frequency. This maps well to APIs such as Vulkan and D3D12, and can be
 * emulated on other APIs as well. We define a standard set of resource set
 * numbers, listed below, and a standard set of slots within each set defined
 * in ResourceSlots.
 */
namespace ResourceSets {
    enum ENUM() Value {
        /** Resources for the view the scene is being rendered from. */
        kViewResources = 0,
        /** Resources for the light for the current pass. */
        kLightResources = 1,
        /** Resources for the current material (contents defined by shader). */
        kMaterialResources = 2,
        /** Resources for the entity currently being rendered. */
        kEntityResources = 3,

        kNumResourceSets,
    };
}

/** Standard resource slot numbers. */
namespace ResourceSlots {
    enum ENUM() Value {
        /** Uniform buffer (this is slot 0 for all sets with uniforms). */
        kUniforms = 0,

        /**
         * Per-entity resources (kEntityResources).
         */

        kNumEntityResources = 1,

        /**
         * Per-view resources (kViewResources).
         */

        kNumViewResources = 1,

        /**
         * Per-light resources (kLightResources).
         */

        /** Shadow map. */
        kShadowMap = 1,

        kNumLightResources = 2,
    };
}
