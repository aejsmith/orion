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
    /* Get a list of all lights affecting the view. */
    m_scene->visitVisibleLights(m_view, [this] (SceneLight *light) {
        addLight(light);
    });

    /* Find all visible entities and add them to all appropriate draw lists. */
    m_scene->visitVisibleEntities(m_view, [this] (SceneEntity *entity) {
        addEntity(entity);
    });

    /* Set view uniforms once per frame. */
    g_gpu->bindUniformBuffer(UniformSlots::kViewUniforms, m_view->uniforms());

    /* For entities with basic materials and for the first light, we don't want
     * blending. Depth buffer writes should be on. FIXME: These should be
     * defaults for the GPU interface here and when this is called this should
     * be expected to be the current state. */
    g_gpu->setBlendState<>();
    g_gpu->setDepthStencilState<>();

    /* Render all entities with basic materials. */
    m_basicDrawList.draw();

    /* Now render lit entities for each light to accumulate all lighting
     * contributions. */
    for (LightRenderState &lightState : m_lights) {
        g_gpu->bindUniformBuffer(UniformSlots::kLightUniforms, lightState.light->uniforms());

        /* Draw all entities. */
        lightState.drawList.draw(lightState.light);

        /* After the first iteration, we want to blend the remaining lights, and
         * we can turn depth writes off. */
        g_gpu->setBlendState<BlendFunc::kAdd, BlendFactor::kOne, BlendFactor::kOne>();
        g_gpu->setDepthStencilState<ComparisonFunc::kEqual, true>();
    }
}

/** Add a light to the lights list.
 * @param light         Light to add. */
void ForwardRenderer::addLight(SceneLight *light) {
    /* Create a new light list entry. */
    m_lights.emplace_back();
    LightRenderState &state = m_lights.back();
    state.light = light;
}

/** Add an entity to the lights list.
 * @param entity        Entity to add. */
void ForwardRenderer::addEntity(SceneEntity *entity) {
    DrawData drawData;
    entity->drawData(drawData);

    /* Determine whether this is a lit or unlit entity. */
    Shader *shader = drawData.material->shader();
    if (shader->numPasses(Pass::kForwardPass) > 0) {
        /* This entity is affected by lights. Add the entity to the draw list
         * for all lights which affect it. */
        for (LightRenderState &lightState : m_lights) {
            /* TODO: Cull lights which don't affect this entity. However this
             * may not be correct. We blend and turn off depth writes after the
             * first light, but if an entity is not affected by that light it
             * would be rendered incorrectly. */
            lightState.drawList.addDrawCalls(drawData, Pass::kForwardPass, entity->uniforms());
        }
    } else if (shader->numPasses(Pass::kBasicPass) > 0) {
        m_basicDrawList.addDrawCalls(drawData, Pass::kBasicPass, entity->uniforms());
    } else {
        /* Drop the entity (deferred only?). */
        return;
    }
}
