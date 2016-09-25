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
 * @brief               Deferred light volume fragment shader.
 */

#include "lighting.h"

layout(location = 0) out vec4 fragColour;

layout(set = kViewResources, binding = kDeferredBufferA) uniform sampler2D deferredBufferA;
layout(set = kViewResources, binding = kDeferredBufferB) uniform sampler2D deferredBufferB;
layout(set = kViewResources, binding = kDeferredBufferC) uniform sampler2D deferredBufferC;
layout(set = kViewResources, binding = kDeferredBufferD) uniform sampler2D deferredBufferD;

/** Decode the G-Buffer data.
 * @param data          Lighting data structure to fill in. */
void decodeGBuffer(out LightingData data) {
    /* Determine G-Buffer texture coordinates. */
    vec2 size = textureSize(deferredBufferA, 0);
    vec2 texcoord = gl_FragCoord.xy / size;

    /* Sample the textures. */
    vec4 sampleA = texture(deferredBufferA, texcoord);
    vec4 sampleB = texture(deferredBufferB, texcoord);
    vec4 sampleC = texture(deferredBufferC, texcoord);
    vec4 sampleD = texture(deferredBufferD, texcoord);

    /* Normal is scaled into the [0, 1] range, change it back to [-1, 1]. */
    data.normal = (sampleA.rgb * 2.0) - 1.0;

    /* Sample the diffuse colour buffer. */
    data.diffuseColour = sampleB.rgb;

    /* Sample specular colour/exponent. Exponent is stored as reciprocal. */
    data.specularColour = sampleC.rgb;
    data.shininess = 1.0 / sampleC.a;

    /* Sample the depth buffer. */
    float bufferDepth = sampleD.r;

    /*
     * Reconstruct the world space position from the depth buffer value. First
     * calculate the NDC position of this fragment. Note that the G-Buffer may
     * be larger than the viewport so take this into account. Then, we transform
     * that by the inverse of the view-projection matrix, and finally divide
     * that by the resulting w component.
     *
     * Reference:
     *  - http://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
     *  - http://http.developer.nvidia.com/GPUGems3/gpugems3_ch27.html
     *  - http://www.songho.ca/opengl/gl_projectionmatrix.html
     *
     * TODO: Can this be optimised? The matrix multiplication is a bit of a
     * perf killer on Intel HD 3000.
     */
    vec4 ndcPosition = vec4(
        (((gl_FragCoord.xy - view.viewportPosition) / view.viewportSize) * 2.0) - 1.0,
        bufferDepth,
        1.0);
    vec4 homogeneousPosition = view.inverseViewProjection * ndcPosition;
    data.position = homogeneousPosition.xyz / homogeneousPosition.w;
}

void main() {
    /* Decode the G-Buffer data. */
    LightingData data;
    decodeGBuffer(data);

    /* Calculate fragment colour. */
    fragColour = calcLight(data);
}
