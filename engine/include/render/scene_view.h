/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene view class.
 */

#ifndef ORION_RENDER_SCENE_VIEW_H
#define ORION_RENDER_SCENE_VIEW_H

#include "gpu/uniform_buffer.h"

#include "math/rect.h"

/** Per-view uniform buffer structure. */
struct ViewUniforms {
	float view[16];			/**< Viewing matrix. */
	float projection[16];		/**< Projection matrix. */
	float view_projection[16];	/**< Combined view-projection matrix. */
	float position[3], _pad;	/**< Viewing position in world space. */
};

/**
 * A view into a scene.
 *
 * This class represents a view into a scene: a viewing transformation, a
 * projection transformation, and a viewport rectangle. It also holds a uniform
 * buffer containing the view's parameters that can be passed to a shader.
 */
class SceneView {
public:
	SceneView();
	~SceneView();

	void set_transform(const glm::vec3 &position, const glm::quat &orientation);
	void perspective(float fovx, float znear, float zfar);
	void set_viewport(const IntRect &viewport);

	/** @return		Current position. */
	const glm::vec3 &position() const { return m_position; }
	/** @return		Current orientation. */
	const glm::quat &orientation() const { return m_orientation; }

	const glm::mat4 &view();

	/** @return		Horizontal field of view. */
	float fovx() const { return m_fovx; }
	/** @return		Near clipping plane. */
	float znear() const { return m_znear; }
	/** @return		Far clipping plane. */
	float zfar() const { return m_zfar; }

	/** @return		Viewport rectangle. */
	const IntRect &viewport() const { return m_viewport; }
	/** @return		Aspect ratio. */
	float aspect() const { return m_aspect; }

	const glm::mat4 &projection();

	GPUBufferPtr uniforms();
private:
	glm::vec3 m_position;		/**< View position. */
	glm::quat m_orientation;	/**< View orientation. */
	glm::mat4 m_view;		/**< World-to-view matrix. */
	bool m_view_outdated;		/**< Whether the view matrix needs updating. */

	float m_fovx;			/**< Horizontal field of view. */
	float m_znear;			/**< Near clipping plane. */
	float m_zfar;			/**< Far clipping plane. */
	glm::mat4 m_projection;		/**< View-to-projection matrix. */
	bool m_projection_outdated;	/**< Whether the projection matrix needs updating. */

	IntRect m_viewport;		/**< Viewport rectangle in pixels. */
	float m_aspect;			/**< Aspect ratio. */

	/** Uniform buffer containing per-view parameters. */
	DynamicUniformBuffer<ViewUniforms> m_uniforms;
};

#endif /* ORION_RENDER_SCENE_VIEW_H */
