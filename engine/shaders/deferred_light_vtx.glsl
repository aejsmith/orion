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
 * @brief               Deferred light volume vertex shader.
 */

layout(location = kPositionSemantic) in vec3 attribPosition;

void main() {
    #if defined(AMBIENT_LIGHT) || defined(DIRECTIONAL_LIGHT)
        /* Ambient and deferred lights are rendered as fullscreen quads. Use
         * an identity transformation. */
        gl_Position = vec4(attribPosition, 1.0);
    #else
        /* Other light volumes are rendered as geometry in world space. */
        gl_Position = view.viewProjection * light.volumeTransform * vec4(attribPosition, 1.0);
    #endif
}
