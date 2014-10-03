/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene view class.
 */

#pragma once

#include "render/uniform_buffer.h"

/** Per-view uniform buffer structure. */
UNIFORM_STRUCT_BEGIN(ViewUniforms)
	UNIFORM_STRUCT_MEMBER(glm::mat4, view);
	UNIFORM_STRUCT_MEMBER(glm::mat4, projection);
	UNIFORM_STRUCT_MEMBER(glm::mat4, viewProjection);
	UNIFORM_STRUCT_MEMBER(glm::vec3, position);
UNIFORM_STRUCT_END;

/**
 * A view into a scene.
 *
 * This class represents a view into a scene: a viewing transformation, a
 * projection transformation, and a viewport rectangle. It also holds a uniform
 * buffer containing the view's parameters that can be passed to shaders.
 */
class SceneView {
public:
	SceneView();
	~SceneView();

	void setTransform(const glm::vec3 &position, const glm::quat &orientation);
	void perspective(float fov, float zNear, float zFar);
	void setViewport(const IntRect &viewport);

	/** @return		Current position. */
	const glm::vec3 &position() const { return m_position; }
	/** @return		Current orientation. */
	const glm::quat &orientation() const { return m_orientation; }

	const glm::mat4 &view();

	/** @return		Horizontal field of view. */
	float fov() const { return m_fov; }
	/** @return		Near clipping plane. */
	float zNear() const { return m_zNear; }
	/** @return		Far clipping plane. */
	float zFar() const { return m_zFar; }

	/** @return		Viewport rectangle. */
	const IntRect &viewport() const { return m_viewport; }
	/** @return		Aspect ratio. */
	float aspect() const { return m_aspect; }

	const glm::mat4 &projection();

	GPUBuffer *uniforms();
private:
	glm::vec3 m_position;		/**< View position. */
	glm::quat m_orientation;	/**< View orientation. */
	glm::mat4 m_view;		/**< World-to-view matrix. */
	bool m_viewOutdated;		/**< Whether the view matrix needs updating. */

	float m_fov;			/**< Horizontal field of view. */
	float m_zNear;			/**< Near clipping plane. */
	float m_zFar;			/**< Far clipping plane. */
	glm::mat4 m_projection;		/**< View-to-projection matrix. */
	bool m_projectionOutdated;	/**< Whether the projection matrix needs updating. */

	IntRect m_viewport;		/**< Viewport rectangle in pixels. */
	float m_aspect;			/**< Aspect ratio. */

	/** Uniform buffer containing per-view parameters. */
	UniformBuffer<ViewUniforms> m_uniforms;
};
