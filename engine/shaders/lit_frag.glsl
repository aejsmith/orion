/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Forward lighting fragment shader.
 */

#include "lighting.h"

layout(location = 0) in vec3 vtxPosition;
layout(location = 1) in vec3 vtxNormal;
layout(location = 2) in vec2 vtxTexcoord;

layout(location = 0) out vec4 fragColour;

#ifdef TEXTURED
uniform sampler2D diffuseTexture;
#endif

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
