/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Scene renderer class.
 */

#pragma once

#include "render/draw_list.h"

class RenderTarget;
class Scene;
class SceneLight;
class SceneView;

/** Rendering path enumeration. */
enum class RenderPath {
    kForward,                       /**< Forward rendering. */
    kDeferred,                      /**< Deferred lighting. */
};

/** Structure containing light rendering state. */
struct LightRenderState {
    SceneLight *light;              /**< Light being rendered. */
    DrawList drawList;              /**< List of all draw calls affected by this light. */
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

    void renderDeferred();
    void renderForward();

    void setOutputRenderTarget();
    void setDeferredRenderTarget();
private:
    Scene *m_scene;                 /**< Scene being rendered. */
    SceneView *m_view;              /**< View into the scene to render from. */
    RenderTarget *m_target;         /**< Render target. */
    RenderPath m_path;              /**< Rendering path to use. */

    /** Draw lists. */
    DrawList m_basicDrawList;       /**< Basic material draw list (drawn first). */
    DrawList m_deferredDrawList;    /**< Deferred material draw list. */

    /** List of lights affecting the view. */
    std::list<LightRenderState> m_lights;
};
