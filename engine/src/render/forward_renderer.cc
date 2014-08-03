/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Forward rendering scene renderer.
 */

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
	
}
