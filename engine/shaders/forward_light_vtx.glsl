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

layout(location = kPositionSemantic) in vec3 attrib_position;
layout(location = kNormalSemantic) in vec3 attrib_normal;
layout(location = kTexCoordSemantic) in vec2 attrib_texcoord;

layout(location = 0) out vec3 vtx_position;
layout(location = 1) out vec3 vtx_normal;
layout(location = 2) out vec2 vtx_texcoord;

void main() {
	vtx_position = vec3(entity.transform * vec4(attrib_position, 1.0));
	vtx_normal = vec3(entity.transform * vec4(attrib_normal, 0.0));
	vtx_texcoord = attrib_texcoord;

	gl_Position = view.view_projection * entity.transform * vec4(attrib_position, 1.0);
}
