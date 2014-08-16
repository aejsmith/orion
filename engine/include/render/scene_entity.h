/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene entity base class.
 */

#ifndef ORION_RENDER_SCENE_ENTITY_H
#define ORION_RENDER_SCENE_ENTITY_H

#include "gpu/uniform_buffer.h"

#include "math/transform.h"

class Scene;

/** Per-entity uniform buffer structure. */
struct EntityUniforms {
	float transform[16];
	float position[3], _pad1;
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

	// Derpy hack to get things working
	virtual void render() = 0;

	/** @return		Current transformation. */
	const Transform &transform() const { return m_transform; }
	/** @return		Current position. */
	const glm::vec3 &position() const { return m_transform.position(); }
	/** @return		Current orientation. */
	const glm::quat &orientation() const { return m_transform.orientation(); }
	/** @return		Current scale. */
	const glm::vec3 &scale() const { return m_transform.scale(); }

	GPUBufferPtr uniforms();
protected:
	SceneEntity();
private:
	void set_transform(const Transform &transform);
private:
	Transform m_transform;		/**< Transformation of the entity. */

	/** Uniform buffer containing per-entity parameters. */
	DynamicUniformBuffer<EntityUniforms> m_uniforms;

	friend class Scene;
};

#endif /* ORION_RENDER_SCENE_ENTITY_H */
