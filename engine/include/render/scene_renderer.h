/*
 * Copyright (C) 2015-2016 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
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
    /** Structure containing light rendering state. */
    struct LightRenderState {
        SceneLight *light;              /**< Light being rendered. */
        GPUResourceSet *resources;      /**< Flushed resource set from SceneLight::resourcesForDraw(). */
        DrawList drawList;              /**< List of all (forward) draw calls affected by this light. */

        GPUTexture *shadowMap;          /**< Shadow map texture. */

        /** Draw calls for shadow casters visible to this light. */
        DrawList shadowMapDrawLists[SceneLight::kMaxShadowViews];
    };

    void addLight(SceneLight *light);
    void addEntity(SceneEntity *entity);

    void renderShadowMaps();
    void renderDeferred();
    void renderForward();
    void renderDebug();

    void setViewResources(SceneView *view, RenderPath path);
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
