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

#include "engine/debug_manager.h"

#include "render/post_effect.h"
#include "render/primitive_renderer.h"
#include "render/render_manager.h"
#include "render/scene.h"
#include "render/scene_light.h"
#include "render/scene_renderer.h"
#include "render/scene_view.h"

#include "shader/material.h"

/** Initialize the scene renderer.
 * @param scene         Scene to render.
 * @param view          View into the scene to render from.
 * @param target        Initial render target.
 * @param path          Render path to use. */
SceneRenderer::SceneRenderer(Scene *scene, SceneView *view, RenderTarget *target, RenderPath path) :
    m_scene(scene),
    m_view(view),
    m_target(target),
    m_path(path)
{}

/** Destroy the scene renderer. */
SceneRenderer::~SceneRenderer() {}

/** Render the scene. */
void SceneRenderer::render() {
    /* Allocate render targets. */
    g_renderManager->allocRenderTargets(m_path, glm::ivec2(m_target->width(), m_target->height()));

    /* Get all lights affecting the view and set up state for them. */
    m_scene->visitVisibleLights(m_view, [this] (SceneLight *l) { addLight(l); });

    /* Render shadow maps. */
    renderShadowMaps();

    /* Find all visible entities and add them to all appropriate draw lists. */
    m_scene->visitVisibleEntities(m_view, [this] (SceneEntity *e) { addEntity(e); });

    /* Render everything. */
    renderDeferred();
    renderForward();

    /* Perform post-processing, and get the resulting output buffer. When this
     * returns the output buffer it gives should be the current render target. */
    const PostEffectChain *effectChain = m_view->postEffectChain();
    GPUTexture *finalTexture = (effectChain)
        ? effectChain->render(m_view->viewport().size())
        : g_renderManager->renderTargets().colourBuffer.get();

    /* Draw debug primitives. */
    renderDebug();

    /* Finally, blit the output buffer onto the real render target. */
    GPUTextureImageRef source(finalTexture);
    GPUTextureImageRef dest;
    m_target->getTextureImageRef(dest);
    g_gpuManager->blit(
        source,
        dest,
        m_view->viewport().pos(),
        m_view->viewport().pos(),
        m_view->viewport().size());
}

/** Add a light to the light list.
 * @param light         Light to add. */
void SceneRenderer::addLight(SceneLight *light) {
    /* Create a new light list entry. */
    m_lights.emplace_back();
    LightRenderState &state = m_lights.back();
    state.light = light;

    /* Flush pending resource changes, save resource set for later. */
    state.resources = light->resourcesForDraw();

    if (light->castShadows()) {
        /* Allocate a shadow map. */
        state.shadowMap = light->allocShadowMap();

        /* Update the shadow map resource binding. */
        GPUSamplerStatePtr sampler = g_gpuManager->getSamplerState();
        state.resources->bindTexture(ResourceSlots::kShadowMap, state.shadowMap, sampler);

        /* Now find all shadow casting entities which are affected by this light. */
        unsigned numShadowViews = light->numShadowViews();
        for (unsigned i = 0; i < numShadowViews; i++) {
            SceneView *shadowView = light->shadowView(i);

            m_scene->visitVisibleEntities(shadowView, [&] (SceneEntity *entity) {
                if (entity->castShadow()) {
                    state.shadowMapDrawLists[i].addDrawCalls(
                        entity,
                        Pass::Type::kShadowCaster);
                }
            });
        }
    }
}

/** Add an entity to the appropriate draw lists.
 * @param entity        Entity to add. */
void SceneRenderer::addEntity(SceneEntity *entity) {
    Shader *shader = entity->material()->shader();

    /* Determine whether this is a lit or unlit entity. */
    if (m_path == RenderPath::kDeferred && shader->numPasses(Pass::Type::kDeferred) > 0) {
        /* This entity is affected by lights and will be rendered via the
         * deferred path. Add it to the deferred draw list. */
        m_deferredDrawList.addDrawCalls(entity, Pass::Type::kDeferred);
    } else if (shader->numPasses(Pass::Type::kForward) > 0) {
        /* This entity is affected by lights and will be rendered via the
         * forward path. Add the entity to the draw list for all lights which
         * affect it. */
        for (LightRenderState &lightState : m_lights) {
            /* TODO: Cull lights which don't affect this entity. However this
             * may not be correct. We blend and turn off depth writes after the
             * first light, but if an entity is not affected by that light it
             * would be rendered incorrectly. */
            lightState.drawList.addDrawCalls(entity, Pass::Type::kForward);
        }
    } else if (shader->numPasses(Pass::Type::kBasic) > 0) {
        m_basicDrawList.addDrawCalls(entity, Pass::Type::kBasic);
    }
}

/** Render shadow maps. */
void SceneRenderer::renderShadowMaps() {
    GPU_DEBUG_GROUP("Shadow Maps");

    for (LightRenderState &state : m_lights) {
        if (!state.shadowMap)
            continue;

        GPU_DEBUG_GROUP("Light '%s'", state.light->name.c_str());

        /* Need to do a rendering pass for each view. */
        unsigned numShadowViews = state.light->numShadowViews();
        for (unsigned i = 0; i < numShadowViews; i++) {
            GPU_DEBUG_GROUP("View %u", i);

            SceneView *shadowView = state.light->shadowView(i);

            /* Begin a shadow pass. */
            GPURenderPassInstanceDesc passDesc(g_renderManager->resources().sceneShadowMapPass);
            passDesc.targets.depthStencil.texture = state.shadowMap;
            passDesc.targets.depthStencil.layer = i;
            passDesc.clearDepth = 1.0;
            passDesc.renderArea = shadowView->viewport();
            GPUCommandList *cmdList = g_gpuManager->beginRenderPass(passDesc);

            /* Bind resources. */
            cmdList->bindResourceSet(ResourceSets::kLightResources, state.resources);
            setViewResources(cmdList, shadowView, RenderPath::kForward);

            /* Render the shadow map. Default state is what we want here:
             * blending disabled, depth test/write enabled. */
            state.shadowMapDrawLists[i].draw(cmdList);

            g_gpuManager->submitRenderPass(cmdList);
        }
    }
}

/** Perform deferred rendering. */
void SceneRenderer::renderDeferred() {
    if (m_deferredDrawList.empty())
        return;

    GPU_DEBUG_GROUP("Deferred Rendering");

    const RenderManager::RenderTargets &targets = g_renderManager->renderTargets();

    {
        GPU_DEBUG_GROUP("G-Buffer Pass");

        GPURenderPassInstanceDesc deferredPassDesc(g_renderManager->resources().sceneGBufferPass);
        deferredPassDesc.targets.colour[0].texture = targets.deferredBufferA;
        deferredPassDesc.targets.colour[1].texture = targets.deferredBufferB;
        deferredPassDesc.targets.colour[2].texture = targets.deferredBufferC;
        deferredPassDesc.targets.depthStencil.texture = targets.depthBuffer;
        deferredPassDesc.clearColours[0] = glm::vec4(0.0, 0.0, 0.0, 0.0);
        deferredPassDesc.clearColours[1] = glm::vec4(0.0, 0.0, 0.0, 0.0);
        deferredPassDesc.clearColours[2] = glm::vec4(0.0, 0.0, 0.0, 0.0);
        deferredPassDesc.clearDepth = 1.0;
        deferredPassDesc.clearStencil = 0;
        deferredPassDesc.renderArea = m_view->viewport();
        GPUCommandList *cmdList = g_gpuManager->beginRenderPass(deferredPassDesc);

        /* Set view resources. */
        setViewResources(cmdList, m_view, RenderPath::kDeferred);

        /* Render everything to the G-Buffer. Default state is what we want,
         * blending disabled, depth test/write enabled. */
        m_deferredDrawList.draw(cmdList);

        g_gpuManager->submitRenderPass(cmdList);

        /* Make a copy of the depth buffer. We need to do this as we want to
         * keep the same depth buffer while rendering light volumes, but the
         * light shaders need to read the depth buffer. We cannot use a texture
         * as a render target while also sampling it. */
        GPUTextureImageRef source(targets.depthBuffer);
        GPUTextureImageRef dest(targets.deferredBufferD);
        g_gpuManager->blit(source, dest, glm::ivec2(0, 0), glm::ivec2(0, 0), targets.deferredBufferSize);
    }

    {
        GPU_DEBUG_GROUP("Light Pass");

        /* Begin the light pass on the primary render target. */
        GPURenderPassInstanceDesc lightPassDesc(g_renderManager->resources().sceneLightPass);
        lightPassDesc.targets.colour[0].texture = targets.colourBuffer;
        lightPassDesc.targets.depthStencil.texture = targets.depthBuffer;
        // FIXME: Get clear settings from Camera. Wonder if it's worth just doing
        // an explicit clear instead of relying on the render pass, seeing as we
        // currently have the awkwardness of having 2 forward render pass objects
        // depending on whether or not we cleared here.
        lightPassDesc.clearColours[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);
        lightPassDesc.clearDepth = 1.0;
        lightPassDesc.clearStencil = 0;
        lightPassDesc.renderArea = m_view->viewport();
        GPUCommandList *cmdList = g_gpuManager->beginRenderPass(lightPassDesc);

        /* Set view resources. */
        setViewResources(cmdList, m_view, RenderPath::kDeferred);

        /* Render light volumes. */
        cmdList->setBlendState(GPUBlendStateDesc().
            setFunc(BlendFunc::kAdd).
            setSourceFactor(BlendFactor::kOne).
            setDestFactor(BlendFactor::kOne));
        for (LightRenderState &lightState : m_lights) {
            GPU_CMD_DEBUG_GROUP(cmdList, "Light '%s'", lightState.light->name.c_str());

            /* Set up rasterizer/depth testing state. No depth writes here, the
             * light volumes should not affect our depth buffer. FIXME: If Pass ever
             * sets up depth/rasterizer state itself, we need to make sure these
             * don't get overridden. */
            switch (lightState.light->type()) {
                case SceneLight::kAmbientLight:
                case SceneLight::kDirectionalLight:
                    /* These are rendered as full-screen quads and should have their
                     * front faces unconditionally rendered. */
                    cmdList->setDepthStencilState(GPUDepthStencilStateDesc().
                        setDepthFunc(ComparisonFunc::kAlways).
                        setDepthWrite(false));
                    cmdList->setRasterizerState();
                    break;
                default:
                    /* For others we want to render their back faces, so that they
                     * will still be rendered even if the view is inside the light
                     * volume. Test for depth greater than or equal to the back face
                     * of the light volume so that only pixels in front of it are
                     * touched. Additionally, enable depth clamping so that the
                     * light volume is not clipped. */
                    cmdList->setDepthStencilState(GPUDepthStencilStateDesc().
                        setDepthFunc(ComparisonFunc::kGreaterOrEqual).
                        setDepthWrite(false));
                    cmdList->setRasterizerState(GPURasterizerStateDesc().
                        setCullMode(CullMode::kFront).
                        setDepthClamp(true));
                    break;
            }

            /* Set light rendering state. */
            cmdList->bindResourceSet(ResourceSets::kLightResources, lightState.resources);

            /* Draw the light volume. The light volume pass is defined as a forward
             * pass as this is needed for per-light type shader variations to be
             * compiled. */
            Geometry geometry;
            lightState.light->volumeGeometry(geometry);
            DrawList list;
            list.addDrawCalls(
                geometry,
                g_renderManager->resources().deferredLightMaterial,
                nullptr,
                Pass::Type::kForward);
            list.draw(cmdList, lightState.light);
        }

        g_gpuManager->submitRenderPass(cmdList);
    }
}

/** Perform forward rendering. */
void SceneRenderer::renderForward() {
    GPU_DEBUG_GROUP("Forward Rendering");

    const RenderManager::RenderTargets &targets = g_renderManager->renderTargets();

    /* Begin the forward pass. Need to clear if no deferred rendering was done. */
    bool needClear = m_deferredDrawList.empty();
    GPURenderPassInstanceDesc passDesc(
        (needClear)
            ? g_renderManager->resources().sceneForwardClearPass
            : g_renderManager->resources().sceneForwardPass);
    passDesc.targets.colour[0].texture = targets.colourBuffer;
    passDesc.targets.depthStencil.texture = targets.depthBuffer;
    // FIXME: Again, see above regarding clearing.
    passDesc.clearColours[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);
    passDesc.clearDepth = 1.0;
    passDesc.clearStencil = 0;
    passDesc.renderArea = m_view->viewport();
    GPUCommandList *cmdList = g_gpuManager->beginRenderPass(passDesc);

    /* Set view resources. */
    setViewResources(cmdList, m_view, RenderPath::kForward);

    /* Now render lit entities for each light to accumulate all lighting
     * contributions. For the first light, we leave state as default as we don't
     * want blending, and depth buffer writes should be on. */
    for (LightRenderState &lightState : m_lights) {
        if (lightState.drawList.empty())
            continue;

        GPU_DEBUG_GROUP("Light '%s'", lightState.light->name.c_str());

        /* Set light rendering state. */
        cmdList->bindResourceSet(ResourceSets::kLightResources, lightState.resources);

        /* Draw all entities. */
        lightState.drawList.draw(cmdList, lightState.light);

        /* After the first iteration, we want to blend the remaining lights, and
         * we can turn depth writes off. */
        cmdList->setBlendState(GPUBlendStateDesc().
            setFunc(BlendFunc::kAdd).
            setSourceFactor(BlendFactor::kOne).
            setDestFactor(BlendFactor::kOne));
        cmdList->setDepthStencilState(GPUDepthStencilStateDesc().
            setDepthFunc(ComparisonFunc::kEqual).
            setDepthWrite(false));
    }

    {
        /* Render all entities with basic materials. */
        GPU_DEBUG_GROUP("Unlit");
        cmdList->setBlendState();
        cmdList->setDepthStencilState();
        m_basicDrawList.draw(cmdList);
    }

    g_gpuManager->submitRenderPass(cmdList);
}

/** Render debug primitives. */
void SceneRenderer::renderDebug() {
    GPU_DEBUG_GROUP("Debug");

    const RenderManager::RenderTargets &targets = g_renderManager->renderTargets();

    /* Begin the debug primitive pass. */
    GPURenderPassInstanceDesc passDesc(g_renderManager->resources().sceneForwardPass);
    passDesc.targets.colour[0].texture = targets.colourBuffer;
    passDesc.targets.depthStencil.texture = targets.depthBuffer;
    passDesc.renderArea = m_view->viewport();
    GPUCommandList *cmdList = g_gpuManager->beginRenderPass(passDesc);

    setViewResources(cmdList, m_view, RenderPath::kForward);

    /* Draw debug primitives onto the view. */
    g_debugManager->renderView(cmdList, m_view);

    g_gpuManager->submitRenderPass(cmdList);

}

/** Update and set the view resources. */
void SceneRenderer::setViewResources(GPUCommandList *cmdList, SceneView *view, RenderPath path) {
    /* Flush uniform changes and get the resources. */
    GPUResourceSet *resources = view->resourcesForDraw();

    /* Update the deferred buffer bindings if we need them. */
    if (path == RenderPath::kDeferred) {
        const RenderManager::RenderTargets &targets = g_renderManager->renderTargets();

        GPUSamplerStatePtr sampler = g_gpuManager->getSamplerState();

        resources->bindTexture(ResourceSlots::kDeferredBufferA, targets.deferredBufferA, sampler);
        resources->bindTexture(ResourceSlots::kDeferredBufferB, targets.deferredBufferB, sampler);
        resources->bindTexture(ResourceSlots::kDeferredBufferC, targets.deferredBufferC, sampler);
        resources->bindTexture(ResourceSlots::kDeferredBufferD, targets.deferredBufferD, sampler);
    }

    cmdList->bindResourceSet(ResourceSets::kViewResources, resources);
}
