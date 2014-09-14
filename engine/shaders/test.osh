/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Test shader.
 *
 * This is (probably) what shader definition files will look like in future.
 * We can bundle multiple shader stages into a single file and include a bunch
 * of metadata that the loader will use to automatically bind attributes,
 * uniform blocks and samplers. The common definitions will be copied into each
 * stage.
 *
 * We do kind of duplicate the layout qualifier functionality provided by GL
 * here, but our target GL version is 3.3 and this does not support layout
 * qualifiers for uniform blocks and samplers.
 *
 * @todo		We could probably remove the need to declare uniform
 *			blocks here altogether if we have enough metadata in
 *			uniform block declarations in C++ code, we can generate
 *			the definitions in the shader code ourself.
 */

/* Target GLSL version, copied into each generated shader. */
@version 330

/* Custom uniform block type definition. */
@typedef MaterialUniforms {
	float shininess;
	vec3 specular;
}

/*
 * Uniform block definitions, with type and an optional instance name. There are
 * a number of predefined types such as per-light and per-entity parameters. The
 * final field specifies the block binding point, with an optional index (e.g.
 * for the custom block binding range).
 */
@uniforms LightUniforms light : kLightUniforms
@uniforms EntityUniforms entity : kEntityUniforms
@uniforms ViewUniforms view : kViewUniforms
@uniforms MaterialUniforms material : kCustomUniforms[0]

/*
 * Sampler definition. The final field specifies the texture unit. There are
 * some predefined texture units, and a custom range as with the uniform block
 * bindings.
 */
@sampler sampler2D baseTexture : kBaseTexture

/*
 * Beginning of the vertex program. We define the input vertex attributes and
 * the output variables in this section. Inputs in the vertex program must be
 * specified with a semantic.
 */
@program kVertexProgram

@in vec3 attribPosition : kPositionSemantic
@in vec3 attribNormal : kNormalSemantic
@in vec2 attribTexCoord : kTexCoordSemantic[0]

@out vec2 vtxPosition;
@out vec2 vtxNormal;
@out vec2 vtxTexCoord;

void main() {
	vtxPosition = vec3(entity.transform * vec4(attribPosition, 1.0));
	vtxNormal = vec3(entity.transform * vec4(attribNormal, 0.0));
	vtxTexcoord = attribTexcoord;

	gl_Position = view.viewProjection * entity.transform * vec4(attribPosition, 1.0);
}

/*
 * Beginning of the fragment program. If a fragment program is defined in the
 * same file as a vertex program, the input variables will be copied from the
 * vertex program's outputs. Otherwise, the inputs must be manually defined.
 *
 * Since we use separate shader objects, we have to be careful to ensure the
 * interface between stages matches up. Matching is done either based on name,
 * or location qualifier. So that we don't have to match names between shaders
 * in different files, the generated code will have sequentially allocated
 * location qualifiers, so as long as outputs and inputs are defined in the
 * same order (with the same type) they will match up.
 *
 * Outputs from the fragment program must have a buffer index specified.
 */
@program kFragmentProgram

@out vec4 fragColour : 0

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

		/* Do specular reflection using Blinn-Phong. Calculate the
		 * cosine of the angle between the normal and the half vector. */
		vec3 halfVactor = normalize(direction - (vtxPosition - view.position));
		float specularAngle = dot(normal, halfVactor);
		if(specularAngle > 0.0) {
			totalFactor +=
				vec4(light.colour, 1.0) *
				pow(specularAngle, material.shininess) *
				vec4(material.specular, 1.0);
		}
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

	vec4 texel = texture(baseTexture, vtxTexCoord);
	fragColour = texel * lightFactor;
}
