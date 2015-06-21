/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Debug text fragment shader.
 */

layout(location = 0) in vec2 vtxTexcoord;

layout(location = 0) out vec4 fragColour;

uniform sampler2D font;

void main() {
    fragColour = vec4(colour, texture(font, vtxTexcoord).r);
}
