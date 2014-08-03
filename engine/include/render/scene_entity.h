/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene entity base class.
 */

#ifndef ORION_RENDER_SCENE_ENTITY_H
#define ORION_RENDER_SCENE_ENTITY_H

#include "gpu/uniform_buffer.h"

/** Per-entity uniform buffer structure. */
struct EntityUniforms {
	float transform[16];		/**< World transformation matrix. */
	float position[4];		/**< Position of the entity in the world. */
};

/**
 * Base class for a scene entity.
 *
 * This class is the base for a renderable entity in the scene. Each Entity in
 * the world which has a rendering component attached will add one or more
 * SceneEntities to the renderer's scene in order for them to be rendered.
 */
class SceneEntity {
public:
	virtual ~SceneEntity();

	void transform(const glm::vec3 &position, const glm::quat &orientation, const glm::vec3 &scale);

	/** @return		Current position. */
	const glm::vec3 &position() const { return m_position; }
	/** @return		Current orientation. */
	const glm::quat &orientation() const { return m_orientation; }
	/** @return		Current scale. */
	const glm::vec3 &scale() const { return m_scale; }
	/** @return		Current local-to-world transformation. */
	const glm::mat4 &transform() const { return m_transform; }

	GPUBufferPtr uniforms();
protected:
	SceneEntity();
private:
	glm::vec3 m_position;		/**< Position of the entity. */
	glm::quat m_orientation;	/**< Orientation of the entity. */
	glm::vec3 m_scale;		/**< Scale of the entity. */
	glm::mat4 m_transform;		/**< Pre-calculated local-to-world transformation. */

	/** Uniform buffer containing per-entity parameters. */
	DynamicUniformBuffer<EntityUniforms> m_uniforms;
};

#endif /* ORION_RENDER_SCENE_ENTITY_H */
