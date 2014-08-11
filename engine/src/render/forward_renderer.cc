/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Forward rendering scene renderer.
 */

#include "core/engine.h"

#include "render/scene.h"
#include "render/scene_view.h"

#include "forward_renderer.h"

/** Initialize the scene renderer.
 * @param scene		Scene to render.
 * @param target	Render target.
 * @param params	Renderer parameters. */
ForwardRenderer::ForwardRenderer(Scene *scene, RenderTarget *target, const RendererParams &params) :
	SceneRenderer(scene, target, params)
{}

/** Render the scene.
 * @param view		View to render from. */
void ForwardRenderer::render(SceneView *view) {
	g_engine->gpu()->bind_uniform_buffer(1, view->uniforms());

	/* Render all visible entities. */
	m_scene->visit_visible_entities(view, [](SceneEntity *entity) {
		g_engine->gpu()->bind_uniform_buffer(0, entity->uniforms());
		entity->render();
	});
}
