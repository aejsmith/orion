/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene entity base class.
 */

#pragma once

#include "render/uniform_buffer.h"

class Material;
class Scene;

/** Per-entity uniform buffer structure. */
UNIFORM_STRUCT_BEGIN(EntityUniforms)
	UNIFORM_STRUCT_MEMBER(glm::mat4, transform);
	UNIFORM_STRUCT_MEMBER(glm::vec3, position);
UNIFORM_STRUCT_END;

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

	/** @return		Current transformation. */
	const Transform &transform() const { return m_transform; }
	/** @return		Current position. */
	const glm::vec3 &position() const { return m_transform.position(); }
	/** @return		Current orientation. */
	const glm::quat &orientation() const { return m_transform.orientation(); }
	/** @return		Current scale. */
	const glm::vec3 &scale() const { return m_transform.scale(); }
	/** @return		GPU buffer containing entity uniforms. */
	GPUBufferPtr uniforms() const { return m_uniforms.gpu(); }

	/** Get the material for the entity.
	 * @return		Material for the entity. */
	virtual Material *material() const = 0;

	/**
	 * Draw the entity.
	 *
	 * Submit a GPU draw call for the entity. Shader/resource state for the
	 * draw is already set prior to calling this.
	 */
	virtual void draw() const = 0;
protected:
	SceneEntity();
private:
	void setTransform(const Transform &transform);
private:
	Transform m_transform;		/**< Transformation of the entity. */

	/** Uniform buffer containing per-entity parameters. */
	UniformBuffer<EntityUniforms> m_uniforms;

	friend class Scene;
};
