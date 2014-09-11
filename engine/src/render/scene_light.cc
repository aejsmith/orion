/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene light class.
 */

#include "render/scene_light.h"

IMPLEMENT_UNIFORM_STRUCT(LightUniforms);

/**
 * Construct the light.
 *
 * Constructs the light. Note that most light parameters are not initialized,
 * these must be set manually.
 *
 * @param type		Type of the light.
 */
SceneLight::SceneLight(Type type) :
	m_type(type)
{}

/** Destroy the light. */
SceneLight::~SceneLight() {}

/** Set the direction of the light.
 * @param direction	New light direction. */
void SceneLight::setDirection(const glm::vec3 &direction) {
	m_direction = glm::normalize(direction);
	m_uniforms.invalidate();
}

/** Set the colour of the light.
 * @param colour	New light colour. */
void SceneLight::setColour(const glm::vec3 &colour) {
	m_colour = colour;
	m_uniforms.invalidate();
}

/** Set the intensity of the light.
 * @param intensity	New light intensity. */
void SceneLight::setIntensity(float intensity) {
	m_intensity = intensity;
	m_uniforms.invalidate();
}

/** Set the cutoff angle (for spot lights).
 * @param cutoff	New cutoff angle. Must be <= 45 degrees. */
void SceneLight::setCutoff(float cutoff) {
	orionCheck(cutoff <= 45.0f, "Cutoff angle must be <= 45");

	m_cutoff = cutoff;
	m_uniforms.invalidate();
}

/** Set the range of the light (for point/spot lights).
 * @param range		Range of the light. */
void SceneLight::setRange(float range) {
	m_range = range;
	m_uniforms.invalidate();
}

/** Set the attenuation factors (for point/spot lights).
 * @param constant	Constant attenuation factor.
 * @param linear	Linear attenuation factor.
 * @param exp		Exponentional attenuation factor. */
void SceneLight::setAttenuation(float constant, float linear, float exp) {
	m_attenuationConstant = constant;
	m_attenuationLinear = linear;
	m_attenuationExp = exp;
	m_uniforms.invalidate();
}

/** Set the light position (private function called from Scene).
 * @param position	New light position. */
void SceneLight::setPosition(const glm::vec3 &position) {
	m_position = position;
	m_uniforms.invalidate();
}

/** Get the uniform buffer containing the light's parameters.
 * @return		Light uniform buffer. */
GPUBufferPtr SceneLight::uniforms() {
	return m_uniforms.get([this](const GPUBufferMapper<LightUniforms> &uniforms) {
		uniforms->position = m_position;
		uniforms->direction = m_direction;
		uniforms->colour = m_colour;
		uniforms->type = m_type;
		uniforms->intensity = m_intensity;
		uniforms->range = m_range;
		uniforms->attenuationConstant = m_attenuationConstant;
		uniforms->attenuationLinear = m_attenuationLinear;
		uniforms->attenuationExp = m_attenuationExp;

		/* Precompute cosine of cutoff angle to avoid having to
		 * calculate it for every pixel. */
		uniforms->cosCutoff = cosf(glm::radians(m_cutoff));
	});
}
