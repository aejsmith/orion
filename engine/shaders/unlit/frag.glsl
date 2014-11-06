/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Unlit fragment shader.
 */

layout(location = 0) in vec2 vtxTexcoord;

layout(location = 0) out vec4 fragColour;

uniform sampler2D diffuseTexture;

void main() {
	fragColour = texture(diffuseTexture, vtxTexcoord);
}
