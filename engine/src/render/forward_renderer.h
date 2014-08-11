/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Forward rendering scene renderer.
 */

#ifndef ORION_RENDER_FORWARD_RENDERER_H
#define ORION_RENDER_FORWARD_RENDERER_H

#include "render/scene_renderer.h"

/** Scene renderer implementing forward rendering. */
class ForwardRenderer : public SceneRenderer {
public:
	ForwardRenderer(Scene *scene, RenderTarget *target, const RendererParams &params);

	void render(SceneView *view);
};

#endif /* ORION_RENDER_FORWARD_RENDERER_H */
