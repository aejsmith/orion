#version 330

/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Test fragment shader.
 */

in vec4 vtx_diffuse;

layout(location = 0) out vec4 frag_colour;

void main() {
	frag_colour = vtx_diffuse;
}
