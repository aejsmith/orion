/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Camera component.
 */

#ifndef ORION_WORLD_CAMERA_H
#define ORION_WORLD_CAMERA_H

#include "render/scene_renderer.h"
#include "render/scene_view.h"

#include "world/component.h"

/** A view into the world from which the scene will be rendered. */
class CameraComponent : public Component {
public:
	ORION_COMPONENT(Component::kCameraType);
public:
	explicit CameraComponent(Entity *entity);

	/**
	 * Rendering.
	 */

	void set_render_target(RenderTarget *target);
	void set_viewport(const Rect &viewport);
	void set_rendering_path(RenderConfiguration::Path path);

	/** @return		Render target. */
	RenderTarget *render_target() const { return m_render_target; }
	/** @return		Normalized viewport rectangle. */
	const Rect &viewport() const { return m_viewport; }
	/** @return		Rendering path. */
	RenderConfiguration::Path rendering_path() const { return m_render_config.path; }

	void render();

	/**
	 * Viewing manipulation.
	 */

	/** @return		World-to-view matrix. */
	const glm::mat4 &view() { return m_scene_view.view(); }

	/**
	 * Projection manipulation.
	 */

	void perspective(float fovx = 75.0f, float znear = 0.1f, float zfar = 1000.0f);
	void set_fov(float fovx);
	void set_znear(float znear);
	void set_zfar(float zfar);

	/** @return		Horizontal field of view. */
	float fovx() const { return m_scene_view.fovx(); }
	/** @return		Near clipping plane. */
	float znear() const { return m_scene_view.znear(); }
	/** @return		Far clipping plane. */
	float zfar() const { return m_scene_view.zfar(); }

	/** @return		View-to-projection matrix. */
	const glm::mat4 &projection() { return m_scene_view.projection(); }

	// XXX: Temporary.
	SceneView &scene_view() { return m_scene_view; }
protected:
	~CameraComponent();

	void transformed();
	void activated();
	void deactivated();
private:
	void update_viewport();
private:
	SceneView m_scene_view;			/**< Scene view implementing this camera. */

	RenderTarget *m_render_target;		/**< Render target for the camera. */
	Rect m_viewport;			/**< Normalized viewport rectangle. */
	RenderConfiguration m_render_config;	/**< Rendering configuration. */
};

/** Set up a perspective projection.
 * @param fovx		Horizontal field of view, in degrees.
 * @param znear		Distance to near clipping plane.
 * @param zfar		Distance to far clipping plane. */
inline void CameraComponent::perspective(float fovx, float znear, float zfar) {
	m_scene_view.perspective(fovx, znear, zfar);
}

/** Set the horizontal field of view.
 * @param fovx		New horizontal FOV, in degrees. */
inline void CameraComponent::set_fov(float fovx) {
	m_scene_view.perspective(fovx, znear(), zfar());
}

/** Set the near clipping plane.
 * @param znear		New distance to the near clipping plane. */
inline void CameraComponent::set_znear(float znear) {
	m_scene_view.perspective(fovx(), znear, zfar());
}

/** Set the far clipping plane.
 * @param zfar		New distance to the far clipping plane. */
inline void CameraComponent::set_zfar(float zfar) {
	m_scene_view.perspective(fovx(), znear(), zfar);
}

#endif /* ORION_WORLD_CAMERA_H */
