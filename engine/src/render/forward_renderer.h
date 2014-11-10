/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Forward rendering scene renderer.
 */

#pragma once

#include "render/scene_renderer.h"

/** Scene renderer implementing forward rendering. */
class ForwardRenderer : public SceneRenderer {
public:
    ForwardRenderer(Scene *scene, SceneView *view, RenderTarget *target);

    /** @return             Render path this renderer implements. */
    RenderPath path() const override { return RenderPath::kForward; }

    void render() override;
};
