/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene view class.
 */

#ifndef ORION_RENDER_SCENE_VIEW_H
#define ORION_RENDER_SCENE_VIEW_H

#include "gpu/buffer.h"

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
 */
class SceneView {
public:
	explicit SceneView(Scene *scene);
	~SceneView();

	/** @return		Scene that the view is for. */
	Scene *scene() const { return m_scene; }

	void set_transform(const glm::vec3 &position, const glm::quat &orientation);
	void set_projection(const glm::mat4 &projection);

	/** Set the viewport rectangle.
	 * @param viewport	New viewport rectangle (in pixels). */
	void set_viewport(const IntRect &viewport) { m_viewport = viewport; }

	/** @return		View position. */
	const glm::vec3 &position() const { return m_position; }
	/** @return		View orientation. */
	const glm::quat &orientation() const { return m_orientation; }
	/** @return		World-to-view matrix. */
	const glm::mat4 &view() const { return m_view; }
	/** @return		View-to-projection matrix. */
	const glm::mat4 &projection() const { return m_projection; }
	/** @return		Viewport rectangle in pixels. */
	const IntRect &viewport() const { return m_viewport; }

	GPUBufferPtr uniforms();
private:
	Scene *m_scene;			/**< Scene that the view is for. */

	glm::vec3 m_position;		/**< View position. */
	glm::quat m_orientation;	/**< View orientation. */
	glm::mat4 m_view;		/**< World-to-view matrix. */
	glm::mat4 m_projection;		/**< View-to-projection matrix. */
	IntRect m_viewport;		/**< Viewport rectangle in pixels. */

	GPUBufferPtr m_uniforms;	/**< Uniform buffer containing per-view parameters. */
	bool m_uniforms_outdated;	/**< Whether the uniform buffer needs updating. */
};

#endif /* ORION_RENDER_SCENE_VIEW_H */
