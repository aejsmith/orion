/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Render target class.
 */

#include "core/engine.h"

#include "render/render_target.h"
#include "render/scene_view.h"

/** Initialize the render target.
 * @param priority	Rendering priority. */
RenderTarget::RenderTarget(unsigned priority) :
	m_priority(priority)
{}

/** Destroy the render target. */
RenderTarget::~RenderTarget() {
	orion_check(m_views.empty(), "Destroying render target with active scene views");
}

/** Add a scene view to the render target.
 * @param view		View to add. */
void RenderTarget::add_view(SceneView *view) {
	bool was_empty = m_views.empty();
	m_views.push_back(view);

	if(was_empty)
		g_engine->add_render_target(this);
}

/** Remove a scene view from the render target.
 * @param view		View to remove. */
void RenderTarget::remove_view(SceneView *view) {
	m_views.remove(view);

	if(m_views.empty())
		g_engine->remove_render_target(this);
}
