/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Lighting definitions.
 */

#ifndef __LIGHTING_H
#define __LIGHTING_H

#ifdef SHADOW
    #if defined(SPOT_LIGHT)
        uniform sampler2D shadowMap;
    #elif defined(POINT_LIGHT)
        uniform samplerCube shadowMap;
    #endif
#endif

/** Structure containing data for lighting calculation. */
struct LightingData {
    vec3 normal;                    /**< World space normal vector. */
    vec3 position;                  /**< World space position vector. */
    vec3 diffuseColour;             /**< Diffuse colour value. */
    vec3 specularColour;            /**< Specular colour value. */
    float shininess;                /**< Specular exponent. */
};

/** Calculate the shadow factor for the light.
 * @param data          Lighting calculation data.
 * @return              Shadow attenuation factor. */
float calcShadow(LightingData data) {
    #ifdef SHADOW
        #if defined(SPOT_LIGHT)
            /* Calculate shadow space coordinates (biased to [0, 1] range for
             * texture lookup). */
            vec4 shadowPos = light.shadowSpace * vec4(data.position, 1.0);

            /* Calculate depth (from light point of view) of this pixel. */
            float pixelDepth = shadowPos.z / shadowPos.w;

            /* Sample the shadow map. */
            float shadowDepth = textureProj(shadowMap, shadowPos.xyw).r;

            /* Compare with a bias to prevent shadow acne. */
            return shadowDepth < (pixelDepth - 0.005) ? 0.4 : 1.0;
        #elif defined(POINT_LIGHT)
            /* Cube map face is selected by the highest magnitude component in
             * the light to fragment vector. This component is defines the depth
             * of the pixel. */
            vec3 direction = data.position - light.position;
            vec3 absDirection = abs(direction);
            float localDepth = max(absDirection.x, max(absDirection.y, absDirection.z));

            /* Convert this value to a depth value. */
            float far = light.range;
            float near = light.shadowZNear;
            float ndcDepth = ((far + near) / (far - near)) - ((2.0 * far * near) / (far - near) / localDepth);
            float pixelDepth = (ndcDepth + 1.0) / 2.0;

            /* Sample the shadow map. */
            float shadowDepth = texture(shadowMap, direction).r;

            /* Same as above. */
            return shadowDepth < (pixelDepth - 0.005) ? 0.4 : 1.0;
        #else
            return 1.0;
        #endif
    #else
        return 1.0;
    #endif
}

/** Calculate the Blinn-Phong lighting contribution for a light.
 * @param data          Lighting calculation data.
 * @param direction     Direction from the light to the fragment.
 * @return              Calculated pixel colour. */
vec3 calcLightBlinnPhong(LightingData data, vec3 direction) {
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

    return colour;
}

/** Calculate a pixel colour for a light.
 * @param data          Data needed for lighting calculation.
 * @return              Calculated pixel colour. */
vec4 calcLight(LightingData data) {
    #if defined(AMBIENT_LIGHT)
        return vec4(data.diffuseColour * light.colour * light.intensity, 1.0);
    #else
        vec3 direction;
        float attenuation = 1.0;

        #if defined(DIRECTIONAL_LIGHT)
            /* Same direction for all pixels. */
            direction = light.direction;
        #else
            /* Calculate distance to light and direction to the fragment. */
            vec3 lightToFragment = data.position - light.position;
            float distance = length(lightToFragment);
            direction = normalize(lightToFragment);

            /* Ignore pixels out of range. This gives 1 if the pixel is in range,
             * 0 otherwise. This is in fact significantly faster than using a
             * branch to return if the light is not in range. */
            attenuation = clamp(floor(light.range / distance), 0.0, 1.0);

            #ifdef SPOT_LIGHT
                /* Soften the cone edge. */
                float spotFactor = max(dot(direction, light.direction), light.cosCutoff);
                attenuation *= 1.0 - ((1.0 - spotFactor) / (1.0 - light.cosCutoff));
            #endif

            /* Apply attenuation. */
            attenuation /=
                light.attenuationConstant +
                (light.attenuationLinear * distance) +
                (light.attenuationExp * distance * distance);
        #endif

        /* Apply shadows. */
        attenuation *= calcShadow(data);

        return vec4(calcLightBlinnPhong(data, direction) * attenuation, 1.0);
    #endif
}

#endif /* __LIGHTING_H */
