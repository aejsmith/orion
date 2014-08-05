/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene entity base class.
 */

#include "render/scene_entity.h"

/** Initialize the entity. */
SceneEntity::SceneEntity() :
	scene(nullptr)
{}

/** Destroy the entity. */
SceneEntity::~SceneEntity() {}

/** Private function called from Scene to set the transformation.
 * @param transform	New transformation. */
void SceneEntity::set_transform(const Transform &transform) {
	m_transform = transform;
	m_uniforms.invalidate();
}

/** Get the uniform buffer containing entity parameters.
 * @return		Uniform buffer containing entity parameters. */
GPUBufferPtr SceneEntity::uniforms() {
	return m_uniforms.get([this](const GPUBufferMapper<EntityUniforms> &uniforms) {
		memcpy(&uniforms->transform, glm::value_ptr(m_transform.matrix()), sizeof(uniforms->transform));
		memcpy(&uniforms->position, glm::value_ptr(m_transform.position()), sizeof(uniforms->position));
	});
}
