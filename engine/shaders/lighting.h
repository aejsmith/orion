/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Lighting definitions.
 */

#ifndef __LIGHTING_H
#define __LIGHTING_H

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
    vec3 toLight = -direction;
    vec3 toView = normalize(view.position - data.position);

    /* Calculate the cosine of the angle between the normal and the light
     * direction. If the surface is facing away from the light this will be <= 0. */
    float angle = max(dot(data.normal, toLight), 0.0);

    /* Calculate the diffuse contribution. */
    vec3 colour = (data.diffuseColour * light.colour) * (light.intensity * angle);

    /* Do specular reflection using Blinn-Phong. Calculate the cosine of the
     * angle between the normal and the half vector. */
    vec3 halfVector = normalize(toLight + toView);
    float specularAngle = max(dot(data.normal, halfVector), 0.0);
    colour += (data.specularColour * light.colour) * (light.intensity * pow(specularAngle, data.shininess));

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

        /* Ignore pixels out of range. This gives 1 if the pixel is in range, 0
         * otherwise. This is in fact significantly faster than using a branch
         * to return if the light is not in range. */
        float attenuation = clamp(floor(light.range / distance), 0.0, 1.0);

        #ifdef SPOT_LIGHT
            float spotFactor = dot(direction, light.direction);
            if (spotFactor < light.cosCutoff)
                return vec4(0.0);

            /* Soften the cone edge. */
            attenuation *= 1.0 - (1.0 - spotFactor) * (1.0 / (1.0 - light.cosCutoff));
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

#endif /* __LIGHTING_H */
