/*
 * Copyright (C) 2015 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               Lighting definitions.
 */

#ifndef __LIGHTING_H
#define __LIGHTING_H

#ifdef SHADOW
    /** Shadow sampler. */
    #if defined(SPOT_LIGHT)
        layout(set = kLightResources, binding = kShadowMap) uniform sampler2DShadow shadowMap;
    #elif defined(POINT_LIGHT)
        layout(set = kLightResources, binding = kShadowMap) uniform samplerCubeShadow shadowMap;
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

            /* Calculate texture coordinate in X/Y and reference depth value
             * (depth of this pixel from light point of view). */
            vec3 uvDepth = shadowPos.xyz / shadowPos.w;

            /* Apply bias. */
            uvDepth.z += light.shadowBiasConstant;

            /* 3x3 PCF. */
            float shadow = 0.0;
            vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    vec3 p = vec3(
                        uvDepth.xy + (vec2(x, y) * texelSize),
                        uvDepth.z);

                    /* Sample the shadow map. Returns [0, 1] where 0 is fully
                     * shadowed and 1 is unshadowed. */
                    shadow += texture(shadowMap, p);
                }
            }

            shadow /= 9.0;

            return shadow;
        #elif defined(POINT_LIGHT)
            /* Cube map face is selected by the highest magnitude component in
             * the light to fragment vector. This component defines the depth
             * of the pixel. Also choose the offset directions to use for PCF
             * based on this (we want to vary the other 2 components). */
            vec3 direction = data.position - light.position;
            vec3 absDirection = abs(direction);

            float localDepth;
            vec3 offsetA, offsetB;

            if (absDirection.x >= absDirection.y && absDirection.x >= absDirection.z) {
                localDepth = absDirection.x;
                offsetA = vec3(0.0, 1.0, 0.0);
                offsetB = vec3(0.0, 0.0, 1.0);
            } else if (absDirection.y >= absDirection.x && absDirection.y >= absDirection.z) {
                localDepth = absDirection.y;
                offsetA = vec3(1.0, 0.0, 0.0);
                offsetB = vec3(0.0, 0.0, 1.0);
            } else {
                localDepth = absDirection.z;
                offsetA = vec3(1.0, 0.0, 0.0);
                offsetB = vec3(0.0, 1.0, 0.0);
            }

            /* Convert this value to a depth value. */
            float far = light.range;
            float near = light.shadowZNear;
            float depth = -(far / (near - far)) - ((far * near) / (far - near) / localDepth);

            /* Determine the offsets to use for PCF (a single texel). Multiply
             * by 2 since cube texture coordinates are remapped from the [-1, 1]
             * range to [0, 1]. */
            float texelSize = (1.0 / textureSize(shadowMap, 0).x);
            offsetA = offsetA * texelSize * 2.0;
            offsetB = offsetB * texelSize * 2.0;

            /* Base texture coordinates, with depth comparison value in last
             * component. Normalize the direction here so that the offset should
             * correspond to a single texel. */
            vec4 directionDepth = vec4(normalize(direction), depth + light.shadowBiasConstant);

            /* 3x3 PCF. */
            float shadow = 0.0;
            for (int a = -1; a <= 1; a++) {
                for (int b = -1; b <= 1; b++) {
                    vec4 p = vec4(
                        directionDepth.xyz + (offsetA * a) + (offsetB * b),
                        directionDepth.w);

                    /* Sample the shadow map. Returns [0, 1] where 0 is fully
                     * shadowed and 1 is unshadowed. */
                    shadow += texture(shadowMap, p);
                }
            }

            shadow /= 9.0;

            return shadow;
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
