/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Unlit fragment shader.
 */

layout(location = 0) in vec2 vtxTexcoord;

layout(location = 0) out vec4 fragColour;

#ifdef TEXTURED
uniform sampler2D diffuseTexture;
#endif

void main() {
	#ifdef TEXTURED
	fragColour = texture(diffuseTexture, vtxTexcoord);
	#else
	fragColour = vec4(diffuseColour, 1.0);
	#endif
}
