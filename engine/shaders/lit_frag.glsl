/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Forward lighting fragment shader.
 */

layout(location = 0) in vec3 vtxPosition;
layout(location = 1) in vec3 vtxNormal;
layout(location = 2) in vec2 vtxTexcoord;

layout(location = 0) out vec4 fragColour;

#ifdef TEXTURED
uniform sampler2D diffuseTexture;
#endif

// Merge notes: enable SPECULAR for deferred version
// handle alpha for forward version (see unity)

/** Structure containing data for lighting calculation. */
struct LightingData {
    vec3 normal;                    /**< World space normal vector. */
    vec3 position;                  /**< World space position vector (reconstructed from depth). */
    vec3 diffuseColour;             /**< Diffuse colour value. */
    vec3 specularColour;            /**< Specular colour value. */
    float shininess;                /**< Specular exponent. */
};

/** Calculate the Blinn-Phong lighting contribution for a light.
 * @param data          Data needed for lighting calculation.
 * @param direction     Direction from the light to the fragment.
 * @param attenuation   Attenuation factor.
 * @return              Calculated pixel colour. */
vec4 calcLightBlinnPhong(LightingData data, vec3 direction, float attenuation) {
    vec3 colour = vec3(0.0);

    /* Calculate the cosine of the angle between the normal and the light
     * direction. If the surface is facing away from the light this will be <= 0. */
    float angle = dot(data.normal, -direction);
    if (angle > 0.0) {
        /* Calculate the diffuse contribution. */
        colour = data.diffuseColour * light.colour * light.intensity * angle;

        #ifdef SPECULAR
            /* Do specular reflection using Blinn-Phong. Calculate the cosine of
             * the angle between the normal and the half vector. */
            vec3 halfVector = normalize(direction - (data.position - view.position));
            float specularAngle = dot(data.normal, halfVector);
            if (specularAngle > 0.0)
                colour += data.specularColour * light.colour * light.intensity * pow(specularAngle, data.shininess);
        #endif
    }

    return vec4(colour * attenuation, 1.0);
}

/** Calculate a pixel colour for a light.
 * @param data          Data needed for lighting calculation.
 * @return              Calculated pixel colour. */
vec4 calcLight(LightingData data) {
    #if defined(AMBIENT_LIGHT)
        return vec4(data.diffuseColour * light.colour * light.intensity, 1.0);
    #elif defined(DIRECTIONAL_LIGHT)
        /* Same direction for all pixels. */
        return calcLightBlinnPhong(data, light.direction, 1.0);
    #elif defined(POINT_LIGHT) || defined(SPOT_LIGHT)
        /* Calculate distance to light and direction to the fragment. */
        vec3 lightToFragment = data.position - light.position;
        float distance = length(lightToFragment);
        vec3 direction = normalize(lightToFragment);

        /* Ignore lights out of range. */
        if (distance > light.range)
            return vec4(0.0);

        #ifdef SPOT_LIGHT
            float spotFactor = dot(direction, light.direction);
            if (spotFactor < light.cosCutoff)
                return vec4(0.0);

            /* Soften the cone edge. */
            float attenuation = 1.0 - (1.0 - spotFactor) * (1.0 / (1.0 - light.cosCutoff));
        #else
            float attenuation = 1.0;
        #endif

        /* Apply attenuation. */
        attenuation /=
            light.attenuationConstant +
            (light.attenuationLinear * distance) +
            (light.attenuationExp * distance * distance);

        /* Calculate the basic lighting value. */
        return calcLightBlinnPhong(data, direction, attenuation);
    #endif
}

void main() {
    /* Fill in lighting data. */
    LightingData data;
    data.normal = normalize(vtxNormal);
    data.position = vtxPosition;

    #ifdef TEXTURED
        data.diffuseColour = texture(diffuseTexture, vtxTexcoord).rgb;
    #else
        data.diffuseColour = diffuseColour;
    #endif

    #ifdef SPECULAR
        data.specularColour = specularColour.rgb;
        data.shininess = shininess;
    #endif

    fragColour = calcLight(data);
}
