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
 * @brief               FXAA fragment shader.
 */

#include "post_effect.h"
#include "utility.h"

layout(location = 0) out vec4 fragColour;

/** Custom luminance function.
 * @param rgba          RGBA colour value.
 * @return              Estimated luminance value. */
float FxaaLuma(vec4 rgba) {
    return luminance(rgba.rgb);
}

/*
 * Set up FXAA parameters. Note: If you change any of these, ensure the path
 * supports FXAA_CUSTOM_LUMA.
 */
#define FXAA_PC                 1
#define FXAA_GLSL_130           1
#define FXAA_QUALITY__PRESET    12
#define FXAA_GATHER4_ALPHA      0
#define FXAA_CUSTOM_LUMA        1
#include "../3rdparty/fxaa/Fxaa3_11.h"

void main() {
    vec2 texcoord = calcSourceCoordinate();

    // FIXME: Not sure if this is correct when colour buffer is larger than
    // what we're rendering.
    vec2 rcpFrame = 1.0f / textureSize(sourceTexture, 0);

    fragColour = FxaaPixelShader(
        texcoord,
        vec4(0.0f, 0.0f, 0.0f, 0.0f),       // FxaaFloat4 fxaaConsolePosPos
        sourceTexture,                      // FxaaTex tex
        sourceTexture,                      // FxaaTex fxaaConsole360TexExpBiasNegOne
        sourceTexture,                      // FxaaTex fxaaConsole360TexExpBiasNegTwo
        rcpFrame,                           // FxaaFloat2 fxaaQualityRcpFrame
        vec4(0.0f, 0.0f, 0.0f, 0.0f),       // FxaaFloat4 fxaaConsoleRcpFrameOpt
        vec4(0.0f, 0.0f, 0.0f, 0.0f),       // FxaaFloat4 fxaaConsoleRcpFrameOpt2
        vec4(0.0f, 0.0f, 0.0f, 0.0f),       // FxaaFloat4 fxaaConsole360RcpFrameOpt2
        0.75f,                              // FxaaFloat fxaaQualitySubpix
        0.166f,                             // FxaaFloat fxaaQualityEdgeThreshold
        0.0833f,                            // FxaaFloat fxaaQualityEdgeThresholdMin
        0.0f,                               // FxaaFloat fxaaConsoleEdgeSharpness
        0.0f,                               // FxaaFloat fxaaConsoleEdgeThreshold
        0.0f,                               // FxaaFloat fxaaConsoleEdgeThresholdMin
        vec4(0.0f, 0.0f, 0.0f, 0.0f)        // FxaaFloat fxaaConsole360ConstDir
    );
}
