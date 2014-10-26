/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Camera component.
 */

#pragma once

#include "engine/render_target.h"

#include "render/scene_renderer.h"
#include "render/scene_view.h"

#include "world/component.h"

/** A view into the world from which the scene will be rendered. */
class Camera : public Component, public RenderLayer {
public:
	DECLARE_COMPONENT(Component::kCameraType);
public:
	explicit Camera(Entity *entity);

	/**
	 * Rendering.
	 */

	void setRenderingPath(RendererParams::Path path);

	/** @return		Rendering path. */
	RendererParams::Path renderingPath() const { return m_rendererParams.path; }

	void render() override;

	/**
	 * Viewing manipulation.
	 */

	/** @return		World-to-view matrix. */
	const glm::mat4 &view() { return m_sceneView.view(); }

	/**
	 * Projection manipulation.
	 */

	void perspective(float fov = 75.0f, float zNear = 0.1f, float zfar = 1000.0f);
	void setFOV(float fov);
	void setZNear(float zNear);
	void setZFar(float zFar);

	/** @return		Horizontal field of view. */
	float fov() const { return m_sceneView.fov(); }
	/** @return		Near clipping plane. */
	float zNear() const { return m_sceneView.zNear(); }
	/** @return		Far clipping plane. */
	float zFar() const { return m_sceneView.zFar(); }

	/** @return		View-to-projection matrix. */
	const glm::mat4 &projection() { return m_sceneView.projection(); }
protected:
	~Camera();

	void transformed() override;
	void activated() override;
	void deactivated() override;
private:
	void viewportChanged() override;
private:
	SceneView m_sceneView;			/**< Scene view implementing this camera. */
	RendererParams m_rendererParams;	/**< Renderer parameters. */
};

/** Set up a perspective projection.
 * @param fovx		Horizontal field of view, in degrees.
 * @param znear		Distance to near clipping plane.
 * @param zfar		Distance to far clipping plane. */
inline void Camera::perspective(float fovx, float znear, float zfar) {
	m_sceneView.perspective(fovx, znear, zfar);
}

/** Set the horizontal field of view.
 * @param fov		New horizontal FOV, in degrees. */
inline void Camera::setFOV(float fov) {
	m_sceneView.perspective(fov, zNear(), zFar());
}

/** Set the near clipping plane.
 * @param zNear		New distance to the near clipping plane. */
inline void Camera::setZNear(float zNear) {
	m_sceneView.perspective(fov(), zNear, zFar());
}

/** Set the far clipping plane.
 * @param zfar		New distance to the far clipping plane. */
inline void Camera::setZFar(float zFar) {
	m_sceneView.perspective(fov(), zNear(), zFar);
}
