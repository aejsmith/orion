/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Unlit vertex shader.
 */

layout(location = kPositionSemantic) in vec3 attribPosition;
layout(location = kTexcoordSemantic) in vec2 attribTexcoord;

layout(location = 0) out vec2 vtxTexcoord;

void main() {
    vtxTexcoord = attribTexcoord;

    gl_Position = view.viewProjection * entity.transform * vec4(attribPosition, 1.0);
}
