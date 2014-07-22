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
 * here, but our target GL version is 4.1 (the latest supported by OS X) and
 * this does not support layout qualifiers for uniform blocks and samplers.
 *
 * With the resource system, a GPUProgram is a single resource. Since we can
 * have multiple GPUPrograms generated from the same source file, we will key
 * program resources with the program type as well as the path.
 */

/* Target GLSL version, copied into each generated shader. */
@version 410

/* Vertex attribute definitions, bound based on semantic and optional index. */
@attribute vec3 attrib_position : kPositionSemantic
@attribute vec3 attrib_normal : kNormalSemantic
@attribute vec2 attrib_texcoord : kTexCoordSemantic[0]

/* Custom uniform block type definition. */
@typedef MaterialParams {
	float shininess;
	vec3 specular;
}

/*
 * Uniform block definitions, with type and an optional instance name. There are
 * a number of predefined types such as per-light and per-object parameters. The
 * final field specifies the block binding point, with an optional index (e.g.
 * for the custom block binding range).
 */
@uniforms LightParams light : kCurrentLightParams
@uniforms ObjectParams object : kCurrentObjectParams
@uniforms CameraParams camera : kCameraParams
@uniforms WorldParams world : kWorldParams
@uniforms MaterialParams material : kCustomParams[0]

/*
 * Sampler definition. The final field specifies the texture unit. There are
 * some predefined texture units, and a custom range as with the uniform block
 * bindings.
 */
@sampler sampler2D texture : kDiffuseTexture

/*
 * Beginning of the vertex program. We define the outputs in this section,
 * which will be automatically copied into the fragment shader if defined in
 * the same file. Fragment shaders can optionally define inputs in a similar
 * fashion, in case they are defined in separate files.
 *
 * Since we use separate shader objects, we have to be careful to ensure the
 * interface between stages matches up. Matching is done either based on name,
 * or location qualifier. So that we don't have to match names between shaders
 * in different files, the generated code will have sequentially allocated
 * location qualifiers, so as long as outputs and inputs are defined in the
 * same order (with the same type) they will match up.
 */
@program kVertexProgram
@out vec2 vtx_position;
@out vec2 vtx_normal;
@out vec2 vtx_texcoord;

void main() {
	vtx_position = vec3(object.transform * vec4(attrib_position, 1.0));
	vtx_normal = vec3(object.transform * vec4(attrib_normal, 0.0));
	vtx_texcoord = attrib_texcoord;

	gl_Position = object.transform * camera.vp_matrix * vec4(attrib_position, 1.0);
}

/*
 * Beginning of the fragment program. Outputs from the fragment program must
 * have a buffer index specified.
 */
@program kFragmentProgram
@out vec4 frag_colour : 0

/* Calculate the lighting contribution of a light. */
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
		vec3 half_vector = normalize(direction - (vtx_position - camera.position));
		float specular_angle = dot(normal, half_vector);
		if(specular_angle > 0.0) {
			total_factor += vec4(light.colour, 1.0)
				* pow(specular_angle, material.shininess)
				* vec4(material.specular, 1.0);
		}
	}

	return total_factor;
}

/* Calculate the lighting contribution of a directional light. */
vec4 directional_light_factor() {
	return calc_light(light.direction);
}

/* Calculate the lighting contribution of a point light. */
vec4 point_light_factor() {
	/* Calculate distance to light and direction to the vertex. */
	vec3 light_to_vertex = vtx_position - light.position;
	float distance = length(light_to_vertex);
	vec3 direction = normalize(light_to_vertex);

	/* Ignore lights out of range. */
	if(distance > light.range)
		return vec4(0.0);

	/* Calculate the basic light factor. */
	vec4 light_factor = calc_light(direction);

	/* Apply attenuation. */
	float attenuation = light.constant + (light.linear * distance) + (light.exp * distance * distance);
	return light_factor / attenuation;
}

/* Calculate the lighting contribution of a spot light. */
vec4 spot_light_factor() {
	vec3 light_to_vertex = normalize(vtx_position - light.position);
	float spot_factor = dot(light_to_vertex, light.direction);

	if(spot_factor > light_cutoff) {
		/* Same as point light calculation, with cone edge softened. */
		float light_factor = point_light_factor();
		return light_factor * (1.0 - (1.0 - spot_factor) * (1.0 / (1.0 - light.cutoff)));
	} else {
		return vec4(0.0);
	}
}

void main() {
	vec4 texel = texture2D(texture, vtx_texcoord);

	/* Determine the light factor for our light source. */
	vec4 light_factor = vec4(0.0);
	if(light.intensity > 0.0) {
		if(light_type == kDirectionalLight) {
			light_factor = directional_light_factor();
		} else if(light_type == kPointLight) {
			light_factor = point_light_factor();
		} else {
			light_factor = spot_light_factor();
		}
	}

	frag_colour = texel * (vec4(world.ambient_colour, 1.0) + light_factor);
}
