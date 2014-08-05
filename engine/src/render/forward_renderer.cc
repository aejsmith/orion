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
 * @param config	Rendering configuration. */
ForwardRenderer::ForwardRenderer(Scene *scene, RenderTarget *target, const RenderConfiguration &config) :
	SceneRenderer(scene, target, config)
{}

/** Render the scene.
 * @param view		View to render from. */
void ForwardRenderer::render(SceneView *view) {
	g_engine->gpu()->bind_uniform_buffer(1, view->uniforms());

	/* Get the list of entities to render. */
	SceneEntityList entities;
	m_scene->find_visible_entities(view, entities);

	/* Render everything. */
	for(SceneEntity *entity : entities) {
		g_engine->gpu()->bind_uniform_buffer(0, entity->uniforms());
		entity->render();
	}
}
