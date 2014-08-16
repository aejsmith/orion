#version 330
#extension GL_ARB_separate_shader_objects : enable

/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Forward lighting vertex shader.
 */

layout(std140) uniform EntityUniforms {
	mat4 transform;
} entity;

layout(std140) uniform ViewUniforms {
	mat4 view;
	mat4 projection;
	mat4 view_projection;
	vec3 position;
} view;

layout(location = 0) in vec3 attrib_position;
layout(location = 2) in vec3 attrib_normal;
layout(location = 14) in vec4 attrib_diffuse;

layout(location = 0) out vec3 vtx_position;
layout(location = 1) out vec3 vtx_normal;
layout(location = 2) out vec4 vtx_diffuse;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	vtx_position = vec3(entity.transform * vec4(attrib_position, 1.0));
	vtx_normal = vec3(entity.transform * vec4(attrib_normal, 0.0));
	vtx_diffuse = attrib_diffuse;

	gl_Position = view.view_projection * entity.transform * vec4(attrib_position, 1.0);
}
