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
 * @brief               Post-processing effect definitions.
 */

#ifndef __POST_EFFECT_H
#define __POST_EFFECT_H

layout(set = kPostEffectResources, binding = kSourceTexture) uniform sampler2D sourceTexture;
layout(set = kPostEffectResources, binding = kDepthBuffer) uniform sampler2D depthBuffer;

/** Calculate the source texture coordinate for the pixel position.
 * @return              Texture coordinate. */
vec2 calcSourceCoordinate() {
    return gl_FragCoord.xy / textureSize(sourceTexture, 0);
}

/** Sample the source texture at the current pixel position.
 * @return              Sampled texture value. */
vec4 sampleSourceTexture() {
    return texture(sourceTexture, calcSourceCoordinate());
}

/** Calculate the depth buffer coordinate for the pixel position.
 * @return              Texture coordinate. */
vec2 calcDepthCoordinate() {
    return gl_FragCoord.xy / textureSize(depthBuffer, 0);
}

/** Sample the scene depth buffer at the current pixel position.
 * @return              Sampled texture value. */
vec4 sampleDepthBuffer() {
    return texture(depthBuffer, calcSourceCoordinate());
}

#endif /* __POST_EFFECT_H */
