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
 * @brief               Tonemapping fragment shader.
 */

#include "post_effect.h"

layout(location = 0) out vec4 fragColour;

/** Simple Reinhard operator. */
vec3 tonemapReinhard(vec3 x) {
    return x / (x + 1.0);
}

vec3 tonemapUncharted2Internal(vec3 x) {
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;

    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

/**
 * Uncharted 2 tonemapping operator.
 *
 * As detailed in the following:
 *  - https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting
 *  - http://filmicworlds.com/blog/filmic-tonemapping-operators/
 */
vec3 tonemapUncharted2(vec3 x) {
    const float exposureBias = 2.0;
    x *= exposureBias;

    return tonemapUncharted2Internal(x) / tonemapUncharted2Internal(vec3(whitePoint));
}

void main() {
    vec4 sourceColour = sampleSourceTexture();

    vec3 colour = sourceColour.rgb;
    colour *= exposure;

    #if 0
        colour = tonemapReinhard(colour);
    #endif

    #if 1
        colour = tonemapUncharted2(colour);
    #endif

    fragColour = vec4(colour, sourceColour.a);
}
