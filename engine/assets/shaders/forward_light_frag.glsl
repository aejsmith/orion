#version 330
#extension GL_ARB_separate_shader_objects : enable

/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Forward lighting fragment shader.
 */

layout(std140) uniform ViewUniforms {
	mat4 view;
	mat4 projection;
	mat4 view_projection;
	vec3 position;
} view;

layout(std140) uniform LightUniforms {
	vec3 position;
	int type;
	vec3 direction;
	float intensity;
	vec3 colour;
	float cos_cutoff;
	float range;
	float attenuation_constant;
	float attenuation_linear;
	float attenuation_exp;
} light;

// FIXME: Define these somewhere to match SceneLight types.
const int kAmbientLight = 0;
const int kDirectionalLight = 1;
const int kPointLight = 2;
const int kSpotLight = 3;

// FIXME: Put these somewhere.
const float mat_shininess = 32.0;
const vec3 mat_specular = vec3(0.5, 0.5, 0.5);

layout(location = 0) in vec3 vtx_position;
layout(location = 1) in vec3 vtx_normal;
layout(location = 2) in vec4 vtx_diffuse;

layout(location = 0) out vec4 frag_colour;

/** Calculate the lighting contribution of a light.
 * @param direction	Direction from the light to the fragment. */
vec4 calc_light(vec3 direction) {
	vec4 total_factor = vec4(0.0);
	vec3 normal = normalize(vtx_normal);

	/* Calculate the cosine of the angle between the normal and the light
	 * direction. If the surface is facing away from the light this will
	 * be <= 0. */
	float angle = dot(normal, -direction);
	if(angle > 0.0) {
		/* Calculate the diffuse contribution. */
		total_factor += vec4(light.colour, 1.0) * light.intensity * angle;

		/* Do specular reflection using Blinn-Phong. Calculate the
		 * cosine of the angle between the normal and the half vector. */
		vec3 half_vector = normalize(direction - (vtx_position - view.position));
		float specular_angle = dot(normal, half_vector);
		if(specular_angle > 0.0) {
			total_factor +=
				vec4(light.colour, 1.0) *
				pow(specular_angle, mat_shininess) *
				vec4(mat_specular, 1.0);
		}
	}

	return total_factor;
}

/** Calculate the lighting contribution of an ambient light. */
vec4 ambient_light_factor() {
	return vec4(light.colour * light.intensity, 1.0);
}

/** Calculate the lighting contribution of a directional light. */
vec4 directional_light_factor() {
	return calc_light(light.direction);
}

/** Calculate the lighting contribution of a point light. */
vec4 point_light_factor() {
	/* Calculate distance to light and direction to the fragment. */
	vec3 light_to_vertex = vtx_position - light.position;
	float distance = length(light_to_vertex);
	vec3 direction = normalize(light_to_vertex);

	/* Ignore lights out of range. */
	if(distance > light.range)
		return vec4(0.0);

	/* Calculate the basic light factor. */
	vec4 light_factor = calc_light(direction);

	/* Apply attenuation. */
	float attenuation =
		light.attenuation_constant +
		(light.attenuation_linear * distance) +
		(light.attenuation_exp * distance * distance);

	return light_factor / attenuation;
}

/** Calculate the lighting contribution of a spot light. */
vec4 spot_light_factor() {
	vec3 light_to_vertex = normalize(vtx_position - light.position);
	float spot_factor = dot(light_to_vertex, light.direction);

	if(spot_factor > light.cos_cutoff) {
		/* Same as point light calculation, with cone edge softened. */
		return point_light_factor() * (1.0 - (1.0 - spot_factor) * (1.0 / (1.0 - light.cos_cutoff)));
	} else {
		return vec4(0.0);
	}
}

void main() {
	/* Determine the light factor for our light source. */
	vec4 light_factor = vec4(0.0);
	switch(light.type) {
	case kAmbientLight:
		light_factor = ambient_light_factor();
		break;
	case kDirectionalLight:
		light_factor = directional_light_factor();
		break;
	case kPointLight:
		light_factor = point_light_factor();
		break;
	case kSpotLight:
		light_factor = spot_light_factor();
		break;
	}

	frag_colour = vtx_diffuse * light_factor;
}
