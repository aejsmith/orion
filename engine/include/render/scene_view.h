/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene view class.
 */

#ifndef ORION_RENDER_SCENE_VIEW_H
#define ORION_RENDER_SCENE_VIEW_H

#include "gpu/uniform_buffer.h"

#include "math/rect.h"

class Scene;

/** Per-view uniform buffer structure. */
struct ViewUniforms {
	float view[16];			/**< Viewing matrix. */
	float projection[16];		/**< Projection matrix. */
	float view_projection[16];	/**< View-projection matrix. */
	float position[3], _pad;	/**< Viewing position in world space. */
};

/**
 * A view into the scene.
 *
 * This class represents a view into a scene. It is the renderer's version of
 * the Camera component. A Camera simply holds a SceneView and updates it as
 * necessary. The separation allows the renderer to avoid the baggage of
 * creating a component whenever it needs to create a camera for internal use.
 *
 * The view/projection matrices and uniform buffer are automatically created
 * as required on first use. For single use SceneViews you can just fill in the
 * members and the matrices will be created when the getter functions are
 * called. For persistent SceneViews (should only be used by Camera), the
 * update_* functions should be called when things are changed so that the
 * calculated matrices will be updated.
 */
struct SceneView {
	Scene *scene;			/**< Scene that the view is for. */

	/** View settings (call update_view() after changing). */
	glm::vec3 position;		/**< View position. */
	glm::quat orientation;		/**< View orientation. */

	/** Projection settings (call update_projection() after changing). */
	float fovx;			/**< Horizontal field of view. */
	float znear;			/**< Near clipping plane. */
	float zfar;			/**< Far clipping plane. */

	/** Viewport rectangle in pixels (call update_projection() after changing). */
	IntRect viewport;
public:
	SceneView();
	~SceneView();

	/** Mark the view matrix as outdated. */
	void update_view() {
		m_view_outdated = true;
		m_uniforms.invalidate();
	}

	/** Mark the projection matrix as outdated. */
	void update_projection() {
		m_projection_outdated = true;
		m_uniforms.invalidate();
	}

	const glm::mat4 &view();
	const glm::mat4 &projection();
	GPUBufferPtr uniforms();
private:
	glm::mat4 m_view;		/**< World-to-view matrix. */
	bool m_view_outdated;		/**< Whether the view matrix needs updating. */
	glm::mat4 m_projection;		/**< View-to-projection matrix. */
	bool m_projection_outdated;	/**< Whether the projection matrix needs updating. */

	/** Uniform buffer containing per-view parameters. */
	DynamicUniformBuffer<ViewUniforms> m_uniforms;
};

#endif /* ORION_RENDER_SCENE_VIEW_H */
