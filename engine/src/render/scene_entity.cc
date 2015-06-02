/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Scene entity base class.
 */

#include "render/scene_entity.h"

IMPLEMENT_UNIFORM_STRUCT(EntityUniforms, "entity", UniformSlots::kEntityUniforms);

/**
 * Initialize the entity.
 *
 * Note that properties are not initialised. They should be initialised by the
 * creator of the entity.
 */
SceneEntity::SceneEntity() {}

/** Destroy the entity. */
SceneEntity::~SceneEntity() {}

/** Set whether the rendered object casts a shadow.
 * @param castShadow    Whether the rendered object casts a shadow. */
void SceneEntity::setCastShadow(bool castShadow) {
    m_castShadow = castShadow;
}

/** Private function called from Scene to set the transformation.
 * @param transform     New transformation. */
void SceneEntity::setTransform(const Transform &transform) {
    m_transform = transform;

    m_uniforms->transform = m_transform.matrix();
    m_uniforms->position = m_transform.position();
}
