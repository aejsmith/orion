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
 * @brief               Lit vertex shader.
 */

layout(location = kPositionSemantic) in vec3 attribPosition;
layout(location = kNormalSemantic) in vec3 attribNormal;
layout(location = kTexcoordSemantic) in vec2 attribTexcoord;

#ifdef NORMALMAP
layout(location = kTangentSemantic) in vec4 attribTangent;
#endif

layout(location = 0) out vec3 vtxPosition;
layout(location = 1) out vec3 vtxNormal;
layout(location = 2) out vec2 vtxTexcoord;

#ifdef NORMALMAP
layout(location = 3) out vec3 vtxTangent;
layout(location = 4) out vec3 vtxBitangent;
#endif

void main() {
    vtxPosition = vec3(entity.transform * vec4(attribPosition, 1.0));
    vtxNormal   = vec3(entity.transform * vec4(attribNormal, 0.0));
    vtxTexcoord = attribTexcoord;

    #ifdef NORMALMAP
        vec3 tangent = attribTangent.xyz;

        /* Calculate tangent and bitangent in world space so we can transform
         * directly from tangent space to world space in the fragment shader.
         * Bitangent is reconstructed from normal and tangent, handedness is
         * stored in W component of the attribute. */
        vtxTangent   = vec3(entity.transform * vec4(tangent, 0.0));
        vtxBitangent = cross(vtxNormal, vtxTangent) * attribTangent.w;
    #endif

    gl_Position = view.viewProjection * entity.transform * vec4(attribPosition, 1.0);
}
