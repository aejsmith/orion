/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Scene renderer class.
 */

#include "forward_renderer.h"

#include "render/scene_renderer.h"

/** Create a scene renderer.
 * @param scene         Scene to render.
 * @param view          View into the scene to render from.
 * @param target        Initial render target.
 * @param path          Rendering path to use. */
SceneRenderer *SceneRenderer::create(Scene *scene, SceneView *view, RenderTarget *target, RenderPath path) {
    // TODO: Fall back when unsupported.
    switch (path) {
        case RenderPath::kDeferred:
            // TODO
        case RenderPath::kForward:
            return new ForwardRenderer(scene, view, target);
        default:
            unreachable();
    }
}

/** Initialize the scene renderer.
 * @param scene         Scene to render.
 * @param view          View into the scene to render from.
 * @param target        Initial render target. */
SceneRenderer::SceneRenderer(Scene *scene, SceneView *view, RenderTarget *target) :
    m_scene(scene),
    m_view(view),
    m_target(target)
{}
