/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Forward lighting fragment shader.
 */

layout(location = 0) in vec3 vtxPosition;
layout(location = 1) in vec3 vtxNormal;
layout(location = 2) in vec2 vtxTexcoord;

layout(location = 0) out vec4 fragColour;

#ifdef TEXTURED
uniform sampler2D diffuseTexture;
#endif

/** Calculate the lighting contribution of a light.
 * @param direction	Direction from the light to the fragment. */
vec4 calcLight(vec3 direction) {
	vec4 totalFactor = vec4(0.0);
	vec3 normal = normalize(vtxNormal);

	/* Calculate the cosine of the angle between the normal and the light
	 * direction. If the surface is facing away from the light this will
	 * be <= 0. */
	float angle = dot(normal, -direction);
	if(angle > 0.0) {
		/* Calculate the diffuse contribution. */
		totalFactor += vec4(light.colour, 1.0) * light.intensity * angle;

		#ifdef SPECULAR

		/* Do specular reflection using Blinn-Phong. Calculate the
		 * cosine of the angle between the normal and the half vector. */
		vec3 halfVactor = normalize(direction - (vtxPosition - view.position));
		float specularAngle = dot(normal, halfVactor);
		if(specularAngle > 0.0) {
			totalFactor +=
				vec4(light.colour, 1.0) *
				pow(specularAngle, shininess) *
				vec4(specularColour, 1.0);
		}

		#endif
	}

	return totalFactor;
}

/** Calculate the lighting contribution of an ambient light. */
vec4 ambientLightFactor() {
	return vec4(light.colour * light.intensity, 1.0);
}

/** Calculate the lighting contribution of a directional light. */
vec4 directionalLightFactor() {
	return calcLight(light.direction);
}

/** Calculate the lighting contribution of a point light. */
vec4 pointLightFactor() {
	/* Calculate distance to light and direction to the fragment. */
	vec3 lightToVertex = vtxPosition - light.position;
	float distance = length(lightToVertex);
	vec3 direction = normalize(lightToVertex);

	/* Ignore lights out of range. */
	if(distance > light.range)
		return vec4(0.0);

	/* Calculate the basic light factor. */
	vec4 lightFactor = calcLight(direction);

	/* Apply attenuation. */
	float attenuation =
		light.attenuationConstant +
		(light.attenuationLinear * distance) +
		(light.attenuationExp * distance * distance);

	return lightFactor / attenuation;
}

/** Calculate the lighting contribution of a spot light. */
vec4 spotLightFactor() {
	vec3 lightToVertex = normalize(vtxPosition - light.position);
	float spotFactor = dot(lightToVertex, light.direction);

	if(spotFactor > light.cosCutoff) {
		/* Same as point light calculation, with cone edge softened. */
		return pointLightFactor() * (1.0 - (1.0 - spotFactor) * (1.0 / (1.0 - light.cosCutoff)));
	} else {
		return vec4(0.0);
	}
}

void main() {
	/* Determine the light factor for our light source. */
	vec4 lightFactor = vec4(0.0);
	switch(light.type) {
	case kAmbientLight:
		lightFactor = ambientLightFactor();
		break;
	case kDirectionalLight:
		lightFactor = directionalLightFactor();
		break;
	case kPointLight:
		lightFactor = pointLightFactor();
		break;
	case kSpotLight:
		lightFactor = spotLightFactor();
		break;
	}

	#ifdef TEXTURED
	vec4 diffuse = texture(diffuseTexture, vtxTexcoord);
	#else
	vec4 diffuse = vec4(diffuseColour, 1.0);
	#endif

	fragColour = diffuse * lightFactor;
}
