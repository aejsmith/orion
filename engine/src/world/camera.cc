/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Camera component.
 *
 * @todo		Recalculate projection and viewport when render target
 *			is resized (add listeners to RenderTarget).
 */

#include "core/engine.h"

#include "render/scene_view.h"

#include "world/camera.h"
#include "world/entity.h"
#include "world/world.h"

/**
 * Construct a default camera.
 *
 * Constructs the camera with a perspective projection with a 75 degree
 * horizontal FOV, near clipping plane of 1.0 and far clipping plane of 1000.0.
 *
 * @param entity	Entity to attach the camera to.
 */
Camera::Camera(Entity *entity) :
	Component(Component::kCameraType, entity),
	m_fovx(75.0f),
	m_znear(1.0f),
	m_zfar(1000.0f),
	m_viewport(0.0f, 0.0f, 1.0f, 1.0f)
{
	m_scene_view = new SceneView(entity->world()->scene());
	update_viewport();
	update_projection();
}

/** Destroy the camera. */
Camera::~Camera() {
	delete m_scene_view;
}

/** Set up a perspective projection.
 * @param fovx		Horizontal field of view, in degrees.
 * @param znear		Distance to near clipping plane.
 * @param zfar		Distance to far clipping plane. */
void Camera::perspective(float fovx, float znear, float zfar) {
	m_fovx = fovx;
	m_znear = znear;
	m_zfar = zfar;
	update_projection();
}

/** Set the horizontal field of view.
 * @param fovx		New horizontal FOV, in degrees. */
void Camera::set_fov(float fovx) {
	m_fovx = fovx;
	update_projection();
}

/** Set the near clipping plane.
 * @param znear		New distance to the near clipping plane. */
void Camera::set_znear(float znear) {
	m_znear = znear;
	update_projection();
}

/** Set the far clipping plane.
 * @param zfar		New distance to the far clipping plane. */
void Camera::set_zfar(float zfar) {
	m_zfar = zfar;
	update_projection();
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

/** Called when the camera transformation is changed. */
void Camera::transformed() {
	m_scene_view->set_transform(entity()->position(), entity()->orientation());
}

/** Called when the camera becomes active in the world. */
void Camera::activated() {
	// XXX: Attach to render target
}

/** Called when the camera becomes inactive in the world. */
void Camera::deactivated() {
	// XXX: Detach from render target
}

/** Update the projection matrix. */
void Camera::update_projection() {
	/* Determine the aspect ratio. */
	float aspect =
		static_cast<float>(m_scene_view->viewport().width) /
		static_cast<float>(m_scene_view->viewport().height);

	/* Convert horizontal field of view to vertical. */
	float fovx = glm::radians(m_fovx);
	float fovy = 2.0f * atanf(tanf(fovx * 0.5f) / aspect);

	m_scene_view->set_projection(glm::perspective(fovy, aspect, m_znear, m_zfar));
}

/** Update the viewport. */
void Camera::update_viewport() {
	/* Calculate real viewport size based on render target dimensions. */
	// XXX: Actually get this.
	int x = m_viewport.x * 1440;
	int y = m_viewport.y * 900;
	int width = m_viewport.width * 1440;
	int height = m_viewport.height * 900;

	m_scene_view->set_viewport(IntRect(x, y, width, height));
}
