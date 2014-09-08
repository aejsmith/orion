/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Render target class.
 */

#include "engine/engine.h"
#include "engine/render_target.h"

/** Initialize the render target.
 * @param priority	Rendering priority. */
RenderTarget::RenderTarget(unsigned priority) :
	m_priority(priority)
{}

/** Destroy the render target. */
RenderTarget::~RenderTarget() {
	orion_check(m_cameras.empty(), "Destroying render target with active cameras");
}

/** Add a camera to the render target.
 * @param camera	Camera to add. */
void RenderTarget::add_camera(CameraComponent *camera) {
	bool was_empty = m_cameras.empty();
	m_cameras.push_back(camera);

	if(was_empty)
		g_engine->add_render_target(this);
}

/** Remove a camera from the render target.
 * @param camera	Camera to remove. */
void RenderTarget::remove_camera(CameraComponent *camera) {
	m_cameras.remove(camera);

	if(m_cameras.empty())
		g_engine->remove_render_target(this);
}
