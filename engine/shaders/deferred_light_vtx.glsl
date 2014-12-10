/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Deferred light volume vertex shader.
 */

layout(location = kPositionSemantic) in vec3 attribPosition;

void main() {
    #if defined(AMBIENT_LIGHT) || defined(DIRECTIONAL_LIGHT)
        /* Ambient and deferred lights are rendered as fullscreen quads. Use
         * an identity transformation. */
        gl_Position = vec4(attribPosition, 1.0);
    #else
        /* Other light volumes are rendered as geometry in world space. */
        gl_Position = view.viewProjection * light.volumeTransform * vec4(attribPosition, 1.0);
    #endif
}
