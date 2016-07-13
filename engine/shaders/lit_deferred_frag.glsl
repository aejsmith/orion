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
 * @brief               Deferred lighting fragment shader.
 */

layout(location = 0) in vec3 vtxPosition;
layout(location = 1) in vec3 vtxNormal;
layout(location = 2) in vec2 vtxTexcoord;

layout(location = 0) out vec4 deferredBufferA;
layout(location = 1) out vec4 deferredBufferB;
layout(location = 2) out vec4 deferredBufferC;

void main() {
    #ifdef TEXTURED
        vec4 diffuse = texture(diffuseTexture, vtxTexcoord);
    #else
        vec4 diffuse = vec4(diffuseColour, 1.0);
    #endif

    /* Normal must be re-normalized as the interpolated normal may not be, and
     * then it must be scaled to be stored in the unorm texture. */
    deferredBufferA.rgb = (normalize(vtxNormal) + 1.0) / 2.0;
    deferredBufferA.a = 1.0;

    /* Write diffuse colour. */
    deferredBufferB.rgb = diffuse.rgb;
    deferredBufferB.a = 1.0;

    /* Write specular colour/exponent. Since the G-Buffer is a unorm texture,
     * values will be clamped between 0.0 and 1.0. Therefore, we store the
     * exponent as its reciprocal. */
    #ifdef SPECULAR
        deferredBufferC.rgb = specularColour.rgb;
        deferredBufferC.a = 1.0 / shininess;
    #else
        deferredBufferC = vec4(0.0, 0.0, 0.0, 1.0);
    #endif
}
