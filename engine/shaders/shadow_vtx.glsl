/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Shadow map vertex shader.
 */

layout(location = kPositionSemantic) in vec3 attribPosition;

void main() {
    gl_Position = view.viewProjection * entity.transform * vec4(attribPosition, 1.0);
}
