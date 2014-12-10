/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Scene light class.
 */

#include "render/render_manager.h"
#include "render/scene_light.h"

IMPLEMENT_UNIFORM_STRUCT(LightUniforms, "light", UniformSlots::kLightUniforms);

/**
 * Construct the light.
 *
 * Constructs the light. Note that most light parameters are not initialized,
 * these must be set manually.
 *
 * @param type          Type of the light.
 */
SceneLight::SceneLight(Type type) :
    m_type(type)
{}

/** Destroy the light. */
SceneLight::~SceneLight() {}

/** Set the direction of the light.
 * @param direction     New light direction. */
void SceneLight::setDirection(const glm::vec3 &direction) {
    m_direction = glm::normalize(direction);
    m_uniforms->direction = m_direction;
    updateVolumeTransform();
}

/** Set the colour of the light.
 * @param colour        New light colour. */
void SceneLight::setColour(const glm::vec3 &colour) {
    m_colour = colour;
    m_uniforms->colour = m_colour;
}

/** Set the intensity of the light.
 * @param intensity     New light intensity. */
void SceneLight::setIntensity(float intensity) {
    m_intensity = intensity;
    m_uniforms->intensity = m_intensity;
}

/** Set the cutoff angle (for spot lights).
 * @param cutoff        New cutoff angle. Must be <= 45 degrees. */
void SceneLight::setCutoff(float cutoff) {
    checkMsg(cutoff <= 45.0f, "Cutoff angle must be <= 45");

    m_cutoff = cutoff;

    /* Shaders use a precomputed cosine of cutoff angle to avoid having to
     * calculate it for every pixel. */
    m_uniforms->cosCutoff = cosf(glm::radians(m_cutoff));

    updateVolumeTransform();
}

/** Set the range of the light (for point/spot lights).
 * @param range         Range of the light. */
void SceneLight::setRange(float range) {
    m_range = range;
    m_uniforms->range = m_range;

    updateVolumeTransform();
}

/** Set the attenuation factors (for point/spot lights).
 * @param constant      Constant attenuation factor.
 * @param linear        Linear attenuation factor.
 * @param exp           Exponentional attenuation factor. */
void SceneLight::setAttenuation(float constant, float linear, float exp) {
    m_attenuationConstant = constant;
    m_attenuationLinear = linear;
    m_attenuationExp = exp;

    m_uniforms->attenuationConstant = m_attenuationConstant;
    m_uniforms->attenuationLinear = m_attenuationLinear;
    m_uniforms->attenuationExp = m_attenuationExp;
}

/** Set the light position (private function called from Scene).
 * @param position      New light position. */
void SceneLight::setPosition(const glm::vec3 &position) {
    m_position = position;
    m_uniforms->position = m_position;
    updateVolumeTransform();
}

/** Get light volume geometry.
 * @return              Sphere vertex/index data. */
void SceneLight::volumeGeometry(GPUVertexData *&vertices, GPUIndexData *&indices) const {
    switch (m_type) {
        case kAmbientLight:
        case kDirectionalLight:
        default:
            g_renderManager->quadGeometry(vertices, indices);
            break;
        case kPointLight:
            g_renderManager->sphereGeometry(vertices, indices);
            break;
        case kSpotLight:
            g_renderManager->coneGeometry(vertices, indices);
            break;
    }
}

/** Update the light volume transformation. */
void SceneLight::updateVolumeTransform() {
    switch (m_type) {
        case kAmbientLight:
        case kDirectionalLight:
        default:
            /* Volume is a full-screen quad. The light volume shader does not
             * use the transformation here, make no change. */
            break;
        case kPointLight:
            /* Volume is a sphere. Geometry has radius of 1, we must scale this
             * to the light's range. */
            m_volumeTransform.set(
                m_position,
                glm::quat(),
                glm::vec3(m_range, m_range, m_range));
            m_uniforms->volumeTransform = m_volumeTransform.matrix();
            break;
        case kSpotLight:
        {
            /* Volume is a cone. Geometry is pointing in the negative Z
             * direction, with a base radius of 1 and a height of 1. We must
             * scale the radius to the cutoff angle and the height to the
             * light's range. */
            float radius = m_range * tanf(glm::radians(m_cutoff));

            /* Calculate the quaternion that rotates the negative Z vector to
             * the direction. TODO: Code duplication. */
            glm::quat orientation;
            if (m_direction == glm::vec3(0.0f, 0.0f, -1.0f)) {
                orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            } else if (m_direction == -glm::vec3(0.0f, 0.0f, -1.0f)) {
                orientation = glm::angleAxis(glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
            } else {
                orientation = glm::quat(
                    1 + glm::dot(glm::vec3(0.0f, 0.0f, -1.0f), m_direction),
                    glm::cross(glm::vec3(0.0f, 0.0f, -1.0f), m_direction));
            }

            m_volumeTransform.set(
                m_position,
                glm::normalize(orientation),
                glm::vec3(radius, radius, m_range));
            m_uniforms->volumeTransform = m_volumeTransform.matrix();
            break;
        }
    }
}
