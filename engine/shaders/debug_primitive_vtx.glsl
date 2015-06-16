/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Debug primitive vertex shader.
 */

layout(location = kPositionSemantic) in vec3 attribPosition;
layout(location = kDiffuseSemantic) in vec4 attribColour;

layout(location = 0) out vec4 vtxColour;

void main() {
    vtxColour = attribColour;

    gl_Position = view.viewProjection * vec4(attribPosition, 1.0);
}
