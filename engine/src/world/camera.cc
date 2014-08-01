/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Camera component.
 *
 * @todo		Recalculate projection and viewport when render target
 *			is resized (add listeners to RenderTarget).
 */

#include "core/engine.h"
#include "core/window.h"

#include "render/render_target.h"
#include "render/scene_view.h"

#include "world/camera.h"
#include "world/entity.h"
#include "world/world.h"

/**
 * Construct a default camera.
 *
 * Constructs the camera with a perspective projection with a 75 degree
 * horizontal FOV, near clipping plane of 1.0 and far clipping plane of 1000.0.
 * The default render target will be the main window.
 *
 * @param entity	Entity to attach the camera to.
 */
Camera::Camera(Entity *entity) :
	Component(Component::kCameraType, entity),
	m_render_target(g_engine->window()),
	m_viewport(0.0f, 0.0f, 1.0f, 1.0f)
{
	/* Initialize the scene view with a default projection and viewport
	 * matching the main window. Transform will be set when transformed()
	 * is called. */
	m_scene_view.scene = entity->world()->scene();
	perspective();
	update_viewport();
}

/** Destroy the camera. */
Camera::~Camera() {}

/** Set up a perspective projection.
 * @param fovx		Horizontal field of view, in degrees.
 * @param znear		Distance to near clipping plane.
 * @param zfar		Distance to far clipping plane. */
void Camera::perspective(float fovx, float znear, float zfar) {
	m_scene_view.fovx = fovx;
	m_scene_view.znear = znear;
	m_scene_view.zfar = zfar;
	m_scene_view.update_projection();
}

/** Set the horizontal field of view.
 * @param fovx		New horizontal FOV, in degrees. */
void Camera::set_fov(float fovx) {
	m_scene_view.fovx = fovx;
	m_scene_view.update_projection();
}

/** Set the near clipping plane.
 * @param znear		New distance to the near clipping plane. */
void Camera::set_znear(float znear) {
	m_scene_view.znear = znear;
	m_scene_view.update_projection();
}

/** Set the far clipping plane.
 * @param zfar		New distance to the far clipping plane. */
void Camera::set_zfar(float zfar) {
	m_scene_view.zfar = zfar;
	m_scene_view.update_projection();
}

/** Set the render target.
 * @param target	New render target. */
void Camera::set_render_target(RenderTarget *target) {
	if(active_in_world())
		m_render_target->remove_view(&m_scene_view);

	m_render_target = target;
	update_viewport();

	if(active_in_world())
		m_render_target->add_view(&m_scene_view);
}

/**
 * Set the viewport.
 *
 * Sets the viewport rectangle. Coordinates are normalized, range from (0, 0)
 * in the top left corner to (1, 1) in the bottom right corner. The actual
 * viewport rectangle is calculated automatically based on the render target's
 * dimensions.
 *
 * @param viewport	Normalized viewport rectangle.
 */
void Camera::set_viewport(const Rect &viewport) {
	m_viewport = viewport;
	update_viewport();
}

/** Update the viewport. */
void Camera::update_viewport() {
	/* Calculate real viewport size based on render target dimensions. */
	glm::ivec2 size = m_render_target->size();
	m_scene_view.viewport.x = m_viewport.x * size.x;
	m_scene_view.viewport.y = m_viewport.y * size.y;
	m_scene_view.viewport.width = m_viewport.width * size.x;
	m_scene_view.viewport.height = m_viewport.height * size.y;

	/* Projection matrix must be recalculated in case aspect changed. */
	m_scene_view.update_projection();
}

/** Called when the camera transformation is changed. */
void Camera::transformed() {
	m_scene_view.position = entity()->position();
	m_scene_view.orientation = entity()->orientation();
	m_scene_view.update_view();
}

/** Called when the camera becomes active in the world. */
void Camera::activated() {
	m_render_target->add_view(&m_scene_view);
}

/** Called when the camera becomes inactive in the world. */
void Camera::deactivated() {
	m_render_target->remove_view(&m_scene_view);
}
