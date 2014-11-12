/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Forward rendering scene renderer.
 */

#include "engine/material.h"

#include "render/scene.h"
#include "render/scene_light.h"
#include "render/scene_view.h"

#include "forward_renderer.h"

/** Initialize the scene renderer.
 * @param scene         Scene to render.
 * @param view          View into the scene to render from.
 * @param target        Initial render target. */
ForwardRenderer::ForwardRenderer(Scene *scene, SceneView *view, RenderTarget *target) :
    SceneRenderer(scene, view, target)
{}

/** Render the scene. */
void ForwardRenderer::render() {
    /* Get a list of all visible entities. */
    std::list<SceneEntity *> entities;
    m_scene->visitVisibleEntities(m_view, [&entities](SceneEntity *e) { entities.push_back(e); });

    /* Get a list of all lights affecting the view. */
    std::list<SceneLight *> lights;
    m_scene->visitVisibleLights(m_view, [&lights](SceneLight *l) { lights.push_back(l); });

    /* Set view uniforms once per frame. */
    g_gpu->bindUniformBuffer(UniformSlots::kViewUniforms, m_view->uniforms());

    /* For the first light we don't need blending, and want depth writes on.
     * FIXME: These should be defaults for the GPU interface here and when
     * this is called this should be expected to be the current state. */
    g_gpu->setBlendState<>();
    g_gpu->setDepthStencilState<>();

    // FIXME: basic materials

    /* Render all visible entities for each light to accumulate the lighting
     * contributions. */
    for (SceneLight *light : lights) {
        g_gpu->bindUniformBuffer(UniformSlots::kLightUniforms, light->uniforms());

        for (SceneEntity *entity : entities) {
            g_gpu->bindUniformBuffer(UniformSlots::kEntityUniforms, entity->uniforms());

            Material *material = entity->material();
            material->shader()->setDrawState(material);
            material->shader()->pass(Pass::kForwardPass, 0)->setDrawState(light);

            entity->draw();
        }

        /* After the first iteration, we want to blend the remaining lights, and
         * we can turn depth writes off. */
        g_gpu->setBlendState<BlendFunc::kAdd, BlendFactor::kOne, BlendFactor::kOne>();
        g_gpu->setDepthStencilState<ComparisonFunc::kEqual, true>();
    }
}
