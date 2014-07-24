#version 330

/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Test vertex shader.
 */

layout(location = 0) in vec3 attrib_position;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	mat4 mvp = mat4(
		1.000000238418579, 0, 0, 0,
		0, 1.600000262260437, 0, 0,
		0, 0, -1.000199913978577, -1,
		0, 0, 9.801979064941406, 10);

	gl_Position = mvp * vec4(attrib_position, 1.0);
}
