/*
 * Copyright (C) 2015 Alex Smith
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
    const RenderManager::RenderTargets &targets = g_renderManager->renderTargets();

    /* Get all lights affecting the view and set up state for them. */
    m_scene->visitVisibleLights(m_view, [this] (SceneLight *l) { addLight(l); });

    /* Render shadow maps. */
    renderShadowMaps();

    /* Find all visible entities and add them to all appropriate draw lists. */
    m_scene->visitVisibleEntities(m_view, [this] (SceneEntity *e) { addEntity(e); });

    /* Set view uniforms once per frame. */
    g_gpuManager->bindUniformBuffer(UniformSlots::kViewUniforms, m_view->uniforms());

    /* Set the primary render target and clear it. FIXME: Better place for clear
     * and clear settings. */
    setOutputRenderTarget();
    g_gpuManager->clear(
        ClearBuffer::kColourBuffer | ClearBuffer::kDepthBuffer | ClearBuffer::kStencilBuffer,
        glm::vec4(0.0, 0.0, 0.0, 1.0), 1.0, 0);

    /* Render everything. */
    renderDeferred();
    renderForward();

    /* Perform post-processing, and get the resulting output buffer. When this
     * returns the output buffer it gives should be the current render target. */
    const PostEffectChain *effectChain = m_view->postEffectChain();
    GPUTexture *finalTexture = (effectChain)
        ? effectChain->render(targets.colourBuffer, targets.depthBuffer, m_view->viewport().size())
        : targets.colourBuffer.get();

    /* Draw debug primitives onto the view. */
    g_debugManager->renderView(m_view);

    /* Finally, blit the output buffer onto the real render target. */
    GPUTextureImageRef source(finalTexture);
    GPUTextureImageRef dest;
    m_target->gpu(dest);
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

    if (light->castShadows()) {
        /* Allocate a shadow map. */
        state.shadowMap = light->allocShadowMap();

        /* Now find all shadow casting entities which are affected by this light. */
        unsigned numShadowViews = light->numShadowViews();
        for (unsigned i = 0; i < numShadowViews; i++) {
            SceneView *shadowView = light->shadowView(i);

            m_scene->visitVisibleEntities(shadowView, [&] (SceneEntity *entity) {
                if (entity->castShadow()) {
                    state.shadowMapDrawLists[i].addDrawCalls(
                        entity,
                        Pass::kShadowCasterPass);
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
    if (m_path == RenderPath::kDeferred && shader->numPasses(Pass::kDeferredPass) > 0) {
        /* This entity is affected by lights and will be rendered via the
         * deferred path. Add it to the deferred draw list. */
        m_deferredDrawList.addDrawCalls(entity, Pass::kDeferredPass);
    } else if (shader->numPasses(Pass::kForwardPass) > 0) {
        /* This entity is affected by lights and will be rendered via the
         * forward path. Add the entity to the draw list for all lights which
         * affect it. */
        for (LightRenderState &lightState : m_lights) {
            /* TODO: Cull lights which don't affect this entity. However this
             * may not be correct. We blend and turn off depth writes after the
             * first light, but if an entity is not affected by that light it
             * would be rendered incorrectly. */
            lightState.drawList.addDrawCalls(entity, Pass::kForwardPass);
        }
    } else if (shader->numPasses(Pass::kBasicPass) > 0) {
        m_basicDrawList.addDrawCalls(entity, Pass::kBasicPass);
    }
}

/** Render shadow maps. */
void SceneRenderer::renderShadowMaps() {
    /* Disable blending, enable depth testing/writing. */
    g_gpuManager->setBlendState<>();
    g_gpuManager->setDepthStencilState<>();
    g_gpuManager->setRasterizerState<>();

    for (LightRenderState &state : m_lights) {
        if (!state.shadowMap)
            continue;

        /* Bind light uniforms. */
        g_gpuManager->bindUniformBuffer(UniformSlots::kLightUniforms, state.light->uniforms());

        /* Need to do a rendering pass for each view. */
        unsigned numShadowViews = state.light->numShadowViews();
        for (unsigned i = 0; i < numShadowViews; i++) {
            SceneView *shadowView = state.light->shadowView(i);

            /* Set view uniforms. */
            g_gpuManager->bindUniformBuffer(UniformSlots::kViewUniforms, shadowView->uniforms());

            /* Set the render target. */
            GPURenderTargetDesc desc;
            desc.numColours = 0;
            desc.depthStencil.texture = state.shadowMap;
            desc.depthStencil.layer = i;
            g_gpuManager->setRenderTarget(&desc, &shadowView->viewport());

            /* Clear it. */
            g_gpuManager->clear(ClearBuffer::kDepthBuffer, glm::vec4(0.0, 0.0, 0.0, 0.0), 1.0, 0);

            /* Render the shadow map. */
            state.shadowMapDrawLists[i].draw();
        }
    }
}

/** Perform deferred rendering. */
void SceneRenderer::renderDeferred() {
    if (m_path != RenderPath::kDeferred || m_deferredDrawList.empty())
        return;

    const RenderManager::RenderTargets &targets = g_renderManager->renderTargets();

    /* Set the render target to the G-Buffer and clear it. */
    setDeferredRenderTarget();
    g_gpuManager->clear(
        ClearBuffer::kColourBuffer | ClearBuffer::kDepthBuffer | ClearBuffer::kStencilBuffer,
        glm::vec4(0.0, 0.0, 0.0, 0.0), 1.0, 0);

    /* Disable blending, enable depth testing/writing. FIXME: State push/pop. */
    g_gpuManager->setBlendState<>();
    g_gpuManager->setDepthStencilState<>();
    g_gpuManager->setRasterizerState<>();

    /* Render everything to the G-Buffer. */
    m_deferredDrawList.draw();

    /* Make a copy of the depth buffer. We need to do this as we want to keep
     * the same depth buffer while rendering light volumes, but the light
     * shaders need to read the depth buffer. We cannot use a texture as a render
     * target while also sampling it. */
    GPUTextureImageRef source(targets.depthBuffer);
    GPUTextureImageRef dest(targets.deferredBufferD);
    g_gpuManager->blit(source, dest, glm::ivec2(0, 0), glm::ivec2(0, 0), targets.deferredBufferSize);

    /* Now restore primary render target and render light volumes. */
    setOutputRenderTarget();
    g_gpuManager->setBlendState<BlendFunc::kAdd, BlendFactor::kOne, BlendFactor::kOne>();
    for (LightRenderState &lightState : m_lights) {
        /* Set up rasterizer/depth testing state. No depth writes here, the
         * light volumes should not affect our depth buffer. FIXME: If Pass ever
         * sets up depth/rasterizer state itself, we need to make sure these
         * don't get overridden. */
        switch (lightState.light->type()) {
            case SceneLight::kAmbientLight:
            case SceneLight::kDirectionalLight:
                /* These are rendered as full-screen quads and should have their
                 * front faces unconditionally rendered. */
                g_gpuManager->setDepthStencilState<ComparisonFunc::kAlways, false>();
                g_gpuManager->setRasterizerState<CullMode::kBack>();
                break;
            default:
                /* For others we want to render their back faces, so that they
                 * will still be rendered even if the view is inside the light
                 * volume. Test for depth greater than or equal to the back face
                 * of the light volume so that only pixels in front of it are
                 * touched. Additionally, enable depth clamping so that the
                 * light volume is not clipped. */
                g_gpuManager->setDepthStencilState<ComparisonFunc::kGreaterOrEqual, false>();
                g_gpuManager->setRasterizerState<CullMode::kFront, true>();
                break;
        }

        setLightState(lightState);

        /* Draw the light volume. The light volume pass is defined as a forward
         * pass as this is needed for per-light type shader variations to be
         * compiled. */
        Geometry geometry;
        lightState.light->volumeGeometry(geometry);
        DrawList list;
        list.addDrawCalls(geometry, g_renderManager->deferredLightMaterial(), nullptr, Pass::kForwardPass);
        list.draw(lightState.light);
    }
}

/** Perform forward rendering. */
void SceneRenderer::renderForward() {
    /* For entities with basic materials and for the first light, we don't want
     * blending. Depth buffer writes should be on. FIXME: These should be
     * defaults for the GPU interface here and when this is called this should
     * be expected to be the current state. */
    g_gpuManager->setBlendState<>();
    g_gpuManager->setDepthStencilState<>();
    g_gpuManager->setRasterizerState<>();

    /* Now render lit entities for each light to accumulate all lighting
     * contributions. */
    for (LightRenderState &lightState : m_lights) {
        if (lightState.drawList.empty())
            continue;

        setLightState(lightState);

        /* Draw all entities. */
        lightState.drawList.draw(lightState.light);

        /* After the first iteration, we want to blend the remaining lights, and
         * we can turn depth writes off. */
        g_gpuManager->setBlendState<BlendFunc::kAdd, BlendFactor::kOne, BlendFactor::kOne>();
        g_gpuManager->setDepthStencilState<ComparisonFunc::kEqual, false>();
    }

    /* Render all entities with basic materials. */
    m_basicDrawList.draw();
}

/** Set the off-screen output buffers as the render target. */
void SceneRenderer::setOutputRenderTarget() {
    const RenderManager::RenderTargets &targets = g_renderManager->renderTargets();

    GPURenderTargetDesc desc;
    desc.numColours = 1;
    desc.colour[0].texture = targets.colourBuffer;
    desc.depthStencil.texture = targets.depthBuffer;
    g_gpuManager->setRenderTarget(&desc, &m_view->viewport());
}

/** Set the G-Buffer as the render target. */
void SceneRenderer::setDeferredRenderTarget() {
    const RenderManager::RenderTargets &targets = g_renderManager->renderTargets();

    GPURenderTargetDesc desc;
    desc.numColours = 3;
    desc.colour[0].texture = targets.deferredBufferA;
    desc.colour[1].texture = targets.deferredBufferB;
    desc.colour[2].texture = targets.deferredBufferC;

    /* We use the primary depth buffer texture here. Once the G-Buffer pass is
     * completed, it is copied into deferredBufferD. */
    desc.depthStencil.texture = targets.depthBuffer;

    g_gpuManager->setRenderTarget(&desc, &m_view->viewport());
}

/** Set up rendering state for a light common to both forward/deferred paths.
 * @param state         Light to set state for. */
void SceneRenderer::setLightState(LightRenderState &state) {
    g_gpuManager->bindUniformBuffer(UniformSlots::kLightUniforms, state.light->uniforms());

    if (state.shadowMap) {
        GPUSamplerStateDesc samplerDesc;
        samplerDesc.filterMode = SamplerFilterMode::kNearest;
        samplerDesc.maxAnisotropy = 1;
        samplerDesc.addressU = samplerDesc.addressV = samplerDesc.addressW = SamplerAddressMode::kClamp;
        GPUSamplerStatePtr sampler = g_gpuManager->getSamplerState(samplerDesc);

        g_gpuManager->bindTexture(TextureSlots::kShadowMap, state.shadowMap, sampler);
    }
}
