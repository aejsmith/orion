/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene light class.
 */

#include "render/scene_light.h"

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
void SceneLight::set_direction(const glm::vec3 &direction) {
	m_direction = glm::normalize(direction);
	m_uniforms.invalidate();
}

/** Set the colour of the light.
 * @param colour	New light colour. */
void SceneLight::set_colour(const glm::vec3 &colour) {
	m_colour = colour;
	m_uniforms.invalidate();
}

/** Set the intensity of the light.
 * @param intensity	New light intensity. */
void SceneLight::set_intensity(float intensity) {
	m_intensity = intensity;
	m_uniforms.invalidate();
}

/** Set the cutoff angle (for spot lights).
 * @param cutoff	New cutoff angle. Must be <= 45 degrees. */
void SceneLight::set_cutoff(float cutoff) {
	orion_check(cutoff <= 45.0f, "Cutoff angle must be <= 45");

	m_cutoff = cutoff;
	m_uniforms.invalidate();
}

/** Set the range of the light (for point/spot lights).
 * @param range		Range of the light. */
void SceneLight::set_range(float range) {
	m_range = range;
	m_uniforms.invalidate();
}

/** Set the attenuation factors (for point/spot lights).
 * @param constant	Constant attenuation factor.
 * @param linear	Linear attenuation factor.
 * @param exp		Exponentional attenuation factor. */
void SceneLight::set_attenuation(float constant, float linear, float exp) {
	m_attenuation_constant = constant;
	m_attenuation_linear = linear;
	m_attenuation_exp = exp;
	m_uniforms.invalidate();
}

/** Set the light position (private function called from Scene).
 * @param position	New light position. */
void SceneLight::set_position(const glm::vec3 &position) {
	m_position = position;
	m_uniforms.invalidate();
}

/** Get the uniform buffer containing the light's parameters.
 * @return		Light uniform buffer. */
GPUBufferPtr SceneLight::uniforms() {
	return m_uniforms.get([this](const GPUBufferMapper<LightUniforms> &uniforms) {
		memcpy(&uniforms->position, glm::value_ptr(m_position), sizeof(uniforms->position));
		memcpy(&uniforms->direction, glm::value_ptr(m_direction), sizeof(uniforms->direction));
		memcpy(&uniforms->colour, glm::value_ptr(m_colour), sizeof(uniforms->colour));
		uniforms->type = m_type;
		uniforms->intensity = m_intensity;
		uniforms->range = m_range;
		uniforms->attenuation_constant = m_attenuation_constant;
		uniforms->attenuation_linear = m_attenuation_linear;
		uniforms->attenuation_exp = m_attenuation_exp;

		/* Precompute cosine of cutoff angle to avoid having to
		 * calculate it for every pixel. */
		uniforms->cos_cutoff = cosf(glm::radians(m_cutoff));
	});
}
