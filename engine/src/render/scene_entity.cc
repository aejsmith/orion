/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene entity base class.
 */

#include "render/scene_entity.h"

IMPLEMENT_UNIFORM_STRUCT(EntityUniforms);

/** Initialize the entity. */
SceneEntity::SceneEntity() {}

/** Destroy the entity. */
SceneEntity::~SceneEntity() {}

/** Private function called from Scene to set the transformation.
 * @param transform	New transformation. */
void SceneEntity::setTransform(const Transform &transform) {
	m_transform = transform;
	m_uniforms.invalidate();
}

/** Get the uniform buffer containing entity parameters.
 * @return		Uniform buffer containing entity parameters. */
GPUBufferPtr SceneEntity::uniforms() {
	return m_uniforms.get([this](const GPUBufferMapper<EntityUniforms> &uniforms) {
		uniforms->transform = m_transform.matrix();
		uniforms->position = m_transform.position();
	});
}
