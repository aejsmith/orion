#version 330

/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Test vertex shader.
 */

layout(std140) uniform EntityUniforms {
	mat4 transform;
} entity;

layout(std140) uniform CameraUniforms {
	mat4 view;
	mat4 projection;
	mat4 view_projection;
} camera;

layout(location = 0) in vec3 attrib_position;
layout(location = 14) in vec4 attrib_diffuse;

out vec4 vtx_diffuse;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	vtx_diffuse = attrib_diffuse;
	gl_Position = camera.view_projection * entity.transform * vec4(attrib_position, 1.0);
}
