/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Scene renderer class.
 */

#pragma once

#include "render/defs.h"
#include "render/draw_list.h"
#include "render/scene_light.h"

class RenderTarget;
class Scene;
class SceneLight;
class SceneView;

/** Structure containing light rendering state. */
struct LightRenderState {
    SceneLight *light;              /**< Light being rendered. */
    DrawList drawList;              /**< List of all (forward) draw calls affected by this light. */

    GPUTexture *shadowMap;          /**< Shadow map texture. */

    /** Draw calls for shadow casters visible to this light. */
    DrawList shadowMapDrawLists[SceneLight::kMaxShadowViews];
};

/**
 * Class which renders a scene.
 *
 * This class renders a scene from a view into a render target. A new instance
 * is created each time a scene is rendered, it stores temporary state for only
 * that rendering pass. Persistent state across rendering passes is stored by
 * Scene/SceneView instead.
 */
class SceneRenderer {
public:
    SceneRenderer(Scene *scene, SceneView *view, RenderTarget *target, RenderPath path);
    ~SceneRenderer();

    /** Set the render target.
     * @param target        New render target. */
    void setTarget(RenderTarget *target) { m_target = target; }

    void render();
private:
    void addLight(SceneLight *light);
    void addEntity(SceneEntity *entity);

    void renderShadowMaps();
    void renderDeferred();
    void renderForward();

    void setOutputRenderTarget();
    void setDeferredRenderTarget();
    void setLightState(LightRenderState &state);
private:
    Scene *m_scene;                 /**< Scene being rendered. */
    SceneView *m_view;              /**< View into the scene to render from. */
    RenderTarget *m_target;         /**< Render target. */
    RenderPath m_path;              /**< Rendering path to use. */

    /** Draw lists. */
    DrawList m_basicDrawList;       /**< Basic material draw list. */
    DrawList m_deferredDrawList;    /**< Deferred material draw list. */

    /** List of lights affecting the view. */
    std::list<LightRenderState> m_lights;
};
