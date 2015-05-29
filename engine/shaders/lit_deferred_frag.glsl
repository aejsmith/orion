/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Deferred lighting fragment shader.
 */

layout(location = 0) in vec3 vtxPosition;
layout(location = 1) in vec3 vtxNormal;
layout(location = 2) in vec2 vtxTexcoord;

layout(location = 0) out vec4 deferredBufferA;
layout(location = 1) out vec4 deferredBufferB;
layout(location = 2) out vec4 deferredBufferC;

#ifdef TEXTURED
uniform sampler2D diffuseTexture;
#endif

void main() {
    #ifdef TEXTURED
        vec4 diffuse = texture(diffuseTexture, vtxTexcoord);
    #else
        vec4 diffuse = vec4(diffuseColour, 1.0);
    #endif

    /* Normal must be re-normalized as the interpolated normal may not be, and
     * then it must be scaled to be stored in the unorm texture. */
    deferredBufferA.rgb = (normalize(vtxNormal) + 1.0) / 2.0;
    deferredBufferA.a = 1.0;

    /* Write diffuse colour. */
    deferredBufferB.rgb = diffuse.rgb;
    deferredBufferB.a = 1.0;

    /* Write specular colour/exponent. Since the G-Buffer is a unorm texture,
     * values will be clamped between 0.0 and 1.0. Therefore, we store the
     * exponent as its reciprocal. */
    #ifdef SPECULAR
        deferredBufferC.rgb = specularColour.rgb;
        deferredBufferC.a = 1.0 / shininess;
    #else
        deferredBufferC = vec4(0.0, 0.0, 0.0, 1.0);
    #endif
}
