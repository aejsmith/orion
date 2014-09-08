/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Forward rendering scene renderer.
 */

#pragma once

#include "render/scene_renderer.h"

/** Scene renderer implementing forward rendering. */
class ForwardRenderer : public SceneRenderer {
public:
	ForwardRenderer(Scene *scene, RenderTarget *target, const RendererParams &params);

	void render(SceneView *view) override;
};
