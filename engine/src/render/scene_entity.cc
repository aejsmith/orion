/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene entity base class.
 */

#include "render/scene_entity.h"

/** Initialize the entity. */
SceneEntity::SceneEntity() :
	m_position(0.0f, 0.0f, 0.0f),
	m_orientation(1.0f, 0.0f, 0.0f, 0.0f),
	m_scale(1.0f, 1.0f, 1.0f)
{}

/** Destroy the entity. */
SceneEntity::~SceneEntity() {}

/** Set the transformation of the entity.
 * @param position	World position.
 * @param orientation	World orientation.
 * @param scale		World scale. */
void SceneEntity::transform(const glm::vec3 &position, const glm::quat &orientation, const glm::vec3 &scale) {
	m_position = position;
	m_orientation = orientation;
	m_scale = scale;

	/* Recalculate our transformation matrix. */
	m_transform =
		glm::translate(glm::mat4(), m_position) *
		glm::mat4_cast(m_orientation) *
		glm::scale(glm::mat4(), m_scale);

	m_uniforms.invalidate();
}

/** Get the uniform buffer containing entity parameters.
 * @return		Uniform buffer containing entity parameters. */
GPUBufferPtr SceneEntity::uniforms() {
	return m_uniforms.get([this](const GPUBufferMapper<EntityUniforms> &uniforms) {
		memcpy(&uniforms->transform, glm::value_ptr(m_transform), sizeof(uniforms->transform));
		memcpy(&uniforms->position, glm::value_ptr(m_position), sizeof(uniforms->position));
	});
}
