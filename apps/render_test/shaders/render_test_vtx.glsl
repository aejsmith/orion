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
 * @brief               Rendering test vertex shader.
 */

#ifdef VBO

layout(location = kPositionSemantic) in vec2 attribPosition;
layout(location = kDiffuseSemantic) in vec4 attribColour;

layout(location = 0) out vec4 vtxColour;

#else

const vec2 vertices[3] = vec2[] (
    vec2(-0.3, -0.4),
    vec2( 0.3, -0.4),
    vec2( 0.0,  0.4)
);

#endif

void main() {
    #ifdef VBO
        vtxColour = attribColour;
        gl_Position = vec4(attribPosition, 0.0, 1.0);
    #else
        gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0);
    #endif
}
