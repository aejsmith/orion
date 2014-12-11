/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Deferred light volume fragment shader.
 */

layout(location = 0) out vec4 fragColour;

uniform sampler2D deferredBufferA;
uniform sampler2D deferredBufferB;
uniform sampler2D deferredBufferC;
uniform sampler2D deferredBufferD;

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
    /* Calculate the cosine of the angle between the normal and the light
     * direction. If the surface is facing away from the light this will be <= 0. */
    float angle = max(dot(data.normal, -direction), 0.0);

    /* Calculate the diffuse contribution. */
    vec3 colour = (data.diffuseColour * light.colour) * (light.intensity * angle);

    /* Do specular reflection using Blinn-Phong. Calculate the cosine of the
     * angle between the normal and the half vector. */
    vec3 halfVector = normalize(-direction + (view.position - data.position));
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

/** Decode the G-Buffer data.
 * @param data          Lighting data structure to fill in. */
void decodeGBuffer(out LightingData data) {
    /* Determine G-Buffer texture coordinates. */
    vec2 size = textureSize(deferredBufferA, 0);
    vec2 texcoord = gl_FragCoord.xy / size;

    /* Sample the textures. */
    vec4 sampleA = texture(deferredBufferA, texcoord);
    vec4 sampleB = texture(deferredBufferB, texcoord);
    vec4 sampleC = texture(deferredBufferC, texcoord);
    vec4 sampleD = texture(deferredBufferD, texcoord);

    /* Normal is scaled into the [0, 1] range, change it back to [-1, 1]. */
    data.normal = (sampleA.rgb * 2.0) - 1.0;

    /* Sample the diffuse colour buffer. */
    data.diffuseColour = sampleB.rgb;

    /* Sample specular colour/exponent. Exponent is stored as reciprocal. */
    data.specularColour = sampleC.rgb;
    data.shininess = 1.0 / sampleC.a;

    /* Sample the depth buffer. */
    float bufferDepth = sampleD.r;

    /*
     * Reconstruct the world space position from the depth buffer value. First
     * calculate the NDC position of this fragment. Note that the G-Buffer may
     * be larger than the viewport so take this into account. Then, we transform
     * that by the inverse of the view-projection matrix, and finally divide
     * that by the resulting w component.
     *
     * Reference:
     *  - http://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
     *  - http://http.developer.nvidia.com/GPUGems3/gpugems3_ch27.html
     *  - http://www.songho.ca/opengl/gl_projectionmatrix.html
     */
    vec4 ndcPosition = vec4(
        (((gl_FragCoord.xy - view.viewportPosition) / view.viewportSize) * 2.0) - 1.0,
        (bufferDepth * 2.0) - 1.0,
        1.0);
    vec4 homogeneousPosition = view.inverseViewProjection * ndcPosition;
    data.position = homogeneousPosition.xyz / homogeneousPosition.w;
}

void main() {
    /* Decode the G-Buffer data. */
    LightingData data;
    decodeGBuffer(data);

    /* Calculate fragment colour. */
    fragColour = calcLight(data);
}
