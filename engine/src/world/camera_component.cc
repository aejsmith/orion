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

#include "world/camera_component.h"
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
CameraComponent::CameraComponent(Entity *entity) :
	Component(Component::kCameraType, entity),
	m_renderTarget(g_mainWindow),
	m_viewport(0.0f, 0.0f, 1.0f, 1.0f)
{
	/* Initialize the scene view with a default projection and a viewport
	 * matching the main window. Its transformation will be set when our
	 * transformed() function is called. */
	perspective();
	updateViewport();
	setRenderingPath(RendererParams::kDeferredPath);
}

/** Destroy the camera. */
CameraComponent::~CameraComponent() {}

/** Set the render target.
 * @param target	New render target. */
void CameraComponent::setRenderTarget(RenderTarget *target) {
	if(activeInWorld())
		m_renderTarget->removeCamera(this);

	m_renderTarget = target;
	updateViewport();

	if(activeInWorld())
		m_renderTarget->addCamera(this);
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
void CameraComponent::setViewport(const Rect &viewport) {
	m_viewport = viewport;
	updateViewport();
}

/**
 * Set the rendering path.
 *
 * Sets the rendering path to use. If the specified path is not supported by
 * the system we are running on, will fall back on the best supported path.
 *
 * @param path		Rendering path to use.
 */
void CameraComponent::setRenderingPath(RendererParams::Path path) {
	// FIXME: Fall back if unsupported.
	m_rendererParams.path = path;
}

/** Render the scene from the camera to its render target. */
void CameraComponent::render() {
	SceneRenderer *renderer = SceneRenderer::create(
		entity()->world()->scene(),
		m_renderTarget,
		m_rendererParams);

	renderer->render(&m_sceneView);
}

/** Update the viewport. */
void CameraComponent::updateViewport() {
	/* Calculate real viewport size based on render target dimensions. */
	glm::ivec2 size = m_renderTarget->size();
	int32_t x = m_viewport.x * static_cast<float>(size.x);
	int32_t y = m_viewport.y * static_cast<float>(size.y);
	int32_t width = m_viewport.width * static_cast<float>(size.x);
	int32_t height = m_viewport.height * static_cast<float>(size.y);
	m_sceneView.setViewport(IntRect(x, y, width, height));
}

/** Called when the camera transformation is changed. */
void CameraComponent::transformed() {
	m_sceneView.setTransform(entity()->position(), entity()->orientation());
}

/** Called when the camera becomes active in the world. */
void CameraComponent::activated() {
	m_renderTarget->addCamera(this);
}

/** Called when the camera becomes inactive in the world. */
void CameraComponent::deactivated() {
	m_renderTarget->removeCamera(this);
}
