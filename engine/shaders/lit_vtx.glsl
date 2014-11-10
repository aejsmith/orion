/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Forward lighting vertex shader.
 */

layout(location = kPositionSemantic) in vec3 attribPosition;
layout(location = kNormalSemantic) in vec3 attribNormal;
layout(location = kTexcoordSemantic) in vec2 attribTexcoord;

layout(location = 0) out vec3 vtxPosition;
layout(location = 1) out vec3 vtxNormal;
layout(location = 2) out vec2 vtxTexcoord;

void main() {
    vtxPosition = vec3(entity.transform * vec4(attribPosition, 1.0));
    vtxNormal = vec3(entity.transform * vec4(attribNormal, 0.0));
    vtxTexcoord = attribTexcoord;

    gl_Position = view.viewProjection * entity.transform * vec4(attribPosition, 1.0);
}
