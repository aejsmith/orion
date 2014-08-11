/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene renderer class.
 */

#include "forward_renderer.h"

#include "render/scene_renderer.h"

/** Create a scene renderer.
 * @param scene		Scene to render.
 * @param target	Render target.
 * @param params	Renderer parameters. */
SceneRenderer *SceneRenderer::create(Scene *scene, RenderTarget *target, const RendererParams &params) {
	switch(params.path) {
	case RendererParams::kDeferredPath:
		// TODO
	case RendererParams::kForwardPath:
		return new ForwardRenderer(scene, target, params);
	default:
		unreachable();
	}
}

/** Initialize the scene renderer.
 * @param scene		Scene to render.
 * @param target	Render target.
 * @param params	Renderer parameters. */
SceneRenderer::SceneRenderer(Scene *scene, RenderTarget *target, const RendererParams &params) :
	m_scene(scene),
	m_target(target),
	m_params(params)
{}
