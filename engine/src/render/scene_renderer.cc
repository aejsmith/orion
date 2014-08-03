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
 * @param config	Rendering configuration. */
SceneRenderer *SceneRenderer::create(Scene *scene, RenderTarget *target, const RenderConfiguration &config) {
	switch(config.path) {
	case RenderConfiguration::kDeferredPath:
		// TODO
	case RenderConfiguration::kForwardPath:
		return new ForwardRenderer(scene, target, config);
	default:
		unreachable();
	}
}

/** Initialize the scene renderer.
 * @param scene		Scene to render.
 * @param target	Render target.
 * @param config	Rendering configuration. */
SceneRenderer::SceneRenderer(Scene *scene, RenderTarget *target, const RenderConfiguration &config) :
	m_scene(scene),
	m_target(target),
	m_config(config)
{}
