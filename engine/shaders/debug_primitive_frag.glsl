/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Debug primitive fragment shader.
 */

layout(location = 0) in vec4 vtxColour;

layout(location = 0) out vec4 fragColour;

void main() {
    fragColour = vtxColour;
}
