/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Debug text vertex shader.
 */

layout(location = kPositionSemantic) in vec3 attribPosition;
layout(location = kTexcoordSemantic) in vec2 attribTexcoord;

layout(location = 0) out vec2 vtxTexcoord;

void main() {
    vtxTexcoord = attribTexcoord;

    gl_Position = vec4(attribPosition.xy, 0.0, 1.0);
}
