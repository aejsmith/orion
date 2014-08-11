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
CameraComponent::CameraComponent(Entity *entity) :
	Component(Component::kCameraType, entity),
	m_render_target(g_engine->window()),
	m_viewport(0.0f, 0.0f, 1.0f, 1.0f)
{
	/* Initialize the scene view with a default projection and a viewport
	 * matching the main window. Its transformation will be set when our
	 * transformed() function is called. */
	perspective();
	update_viewport();
	set_rendering_path(RendererParams::kDeferredPath);
}

/** Destroy the camera. */
CameraComponent::~CameraComponent() {}

/** Set the render target.
 * @param target	New render target. */
void CameraComponent::set_render_target(RenderTarget *target) {
	if(active_in_world())
		m_render_target->remove_camera(this);

	m_render_target = target;
	update_viewport();

	if(active_in_world())
		m_render_target->add_camera(this);
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
void CameraComponent::set_viewport(const Rect &viewport) {
	m_viewport = viewport;
	update_viewport();
}

/**
 * Set the rendering path.
 *
 * Sets the rendering path to use. If the specified path is not supported by
 * the system we are running on, will fall back on the best supported path.
 *
 * @param path		Rendering path to use.
 */
void CameraComponent::set_rendering_path(RendererParams::Path path) {
	// FIXME: Fall back if unsupported.
	m_renderer_params.path = path;
}

/** Render the scene from the camera to its render target. */
void CameraComponent::render() {
	SceneRenderer *renderer = SceneRenderer::create(
		entity()->world()->scene(),
		m_render_target,
		m_renderer_params);

	renderer->render(&m_scene_view);
}

/** Update the viewport. */
void CameraComponent::update_viewport() {
	/* Calculate real viewport size based on render target dimensions. */
	glm::ivec2 size = m_render_target->size();
	int x = m_viewport.x * static_cast<float>(size.x);
	int y = m_viewport.y * static_cast<float>(size.y);
	int width = m_viewport.width * static_cast<float>(size.x);
	int height = m_viewport.height * static_cast<float>(size.y);
	m_scene_view.set_viewport(IntRect(x, y, width, height));
}

/** Called when the camera transformation is changed. */
void CameraComponent::transformed() {
	m_scene_view.set_transform(entity()->position(), entity()->orientation());
}

/** Called when the camera becomes active in the world. */
void CameraComponent::activated() {
	m_render_target->add_camera(this);
}

/** Called when the camera becomes inactive in the world. */
void CameraComponent::deactivated() {
	m_render_target->remove_camera(this);
}
