/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Forward rendering scene renderer.
 */

#pragma once

#include "render/scene_renderer.h"

#include "draw_list.h"

/** Structure containing light rendering state. */
struct LightRenderState {
    SceneLight *light;                  /**< Light being rendered. */
    DrawList drawList;                  /**< List of all draw calls affected by this light. */
};

/** Scene renderer implementing forward rendering. */
class ForwardRenderer : public SceneRenderer {
public:
    ForwardRenderer(Scene *scene, SceneView *view, RenderTarget *target);

    /** @return             Render path this renderer implements. */
    RenderPath path() const override { return RenderPath::kForward; }

    void render() override;
private:
    void addLight(SceneLight *light);
    void addEntity(SceneEntity *entity);
private:
    /** Basic material draw list (drawn first). */
    DrawList m_basicDrawList;

    /** List of lights affecting the view. */
    std::list<LightRenderState> m_lights;
};
