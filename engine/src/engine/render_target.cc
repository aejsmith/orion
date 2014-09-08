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
	orionCheck(m_cameras.empty(), "Destroying render target with active cameras");
}

/** Add a camera to the render target.
 * @param camera	Camera to add. */
void RenderTarget::addCamera(CameraComponent *camera) {
	bool wasEmpty = m_cameras.empty();
	m_cameras.push_back(camera);

	if(wasEmpty)
		g_engine->addRenderTarget(this);
}

/** Remove a camera from the render target.
 * @param camera	Camera to remove. */
void RenderTarget::removeCamera(CameraComponent *camera) {
	m_cameras.remove(camera);

	if(m_cameras.empty())
		g_engine->removeRenderTarget(this);
}
