/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Camera component.
 *
 * @todo		Recalculate projection and viewport when render target
 *			is resized (add listeners to RenderTarget).
 */

#include "engine/render_target.h"
#include "engine/window.h"

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
	Component(Component::kCameraType, entity)
{
	/* Initialize the scene view with a default projection. */
	perspective();

	/* Default to the main window as the render target. */
	setRenderTarget(g_mainWindow);
	setLayerOrder(RenderLayer::kCameraLayerOrder);

	/* Set default rendering parameters. */
	setRenderingPath(RendererParams::kDeferredPath);
}

/** Destroy the camera. */
Camera::~Camera() {}

/**
 * Set the rendering path.
 *
 * Sets the rendering path to use. If the specified path is not supported by
 * the system we are running on, will fall back on the best supported path.
 *
 * @param path		Rendering path to use.
 */
void Camera::setRenderingPath(RendererParams::Path path) {
	// FIXME: Fall back if unsupported.
	m_rendererParams.path = path;
}

/** Render the scene from the camera to its render target. */
void Camera::render() {
	SceneRenderer *renderer = SceneRenderer::create(
		entity()->world()->scene(),
		renderTarget(),
		m_rendererParams);

	renderer->render(&m_sceneView);
}

/** Update the viewport in the SceneView. */
void Camera::viewportChanged() {
	m_sceneView.setViewport(pixelViewport());
}

/** Called when the camera transformation is changed. */
void Camera::transformed() {
	m_sceneView.setTransform(entity()->position(), entity()->orientation());
}

/** Called when the camera becomes active in the world. */
void Camera::activated() {
	registerLayer();
}

/** Called when the camera becomes inactive in the world. */
void Camera::deactivated() {
	unregisterLayer();
}
