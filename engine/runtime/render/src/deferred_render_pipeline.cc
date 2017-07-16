/*
 * Copyright (C) 2017 Alex Smith
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
 * @brief               Deferred rendering pipeline.
 */

#include "engine/asset_manager.h"
#include "engine/debug_manager.h"

#include "render/deferred_render_pipeline.h"
#include "render/render_entity.h"
#include "render/render_light.h"

#include "render_core/geometry.h"

/** Pass type names. */
static const std::string kDeferredPassType      ("Deferred");
static const std::string kShadowCasterPassType  ("ShadowCaster");
static const std::string kDeferredLightPassType ("DeferredLight");

/** Names of light variations. */
static const std::string kLightVariations[RenderLight::kNumTypes] = {
    "AMBIENT_LIGHT",
    "DIRECTIONAL_LIGHT",
    "POINT_LIGHT",
    "SPOT_LIGHT",
};

/** Name of the shadow variation. */
static const std::string kShadowVariation("SHADOW");

/** Default shadow map resolution. */
static const uint16_t kDefaultShadowMapResolution = 512;

/* Register the deferred pass type. */
DEFINE_PASS_TYPE(kDeferredPassType, {});

/* Register the shadow caster pass type. */
DEFINE_PASS_TYPE(kShadowCasterPassType, {});

/* Register the deferred light pass type. Needs a variation per light type. */
DEFINE_PASS_TYPE(kDeferredLightPassType, {
    { kLightVariations[RenderLight::kAmbientLight] },
    { kLightVariations[RenderLight::kDirectionalLight] },
    { kLightVariations[RenderLight::kDirectionalLight], kShadowVariation },
    { kLightVariations[RenderLight::kPointLight] },
    { kLightVariations[RenderLight::kPointLight], kShadowVariation },
    { kLightVariations[RenderLight::kSpotLight] },
    { kLightVariations[RenderLight::kSpotLight], kShadowVariation }
});

/** Global resources for the deferred pipeline. */
GlobalResource<DeferredRenderPipeline::Resources> DeferredRenderPipeline::m_resources;

/** Initialise global deferred rendering resources. */
DeferredRenderPipeline::Resources::Resources() {
    /* Load the light shader. */
    this->lightShader = g_assetManager->load<Shader>("engine/shaders/internal/deferred_light");

    GPURenderPassDesc passDesc;

    /* Create the shadow map pass. */
    passDesc.colourAttachments.resize(0);
    passDesc.depthStencilAttachment               = GPURenderAttachmentDesc();
    passDesc.depthStencilAttachment.format        = kShadowMapFormat;
    passDesc.depthStencilAttachment.loadOp        = GPURenderLoadOp::kClear;
    passDesc.depthStencilAttachment.stencilLoadOp = GPURenderLoadOp::kDontCare;

    this->shadowMapPass = g_gpuManager->createRenderPass(std::move(passDesc));

    /* Create the G-Buffer pass. */
    passDesc.colourAttachments.resize(3);
    passDesc.colourAttachments[0].format          = kDeferredBufferAFormat;
    passDesc.colourAttachments[0].loadOp          = GPURenderLoadOp::kClear;
    passDesc.colourAttachments[1].format          = kDeferredBufferBFormat;
    passDesc.colourAttachments[1].loadOp          = GPURenderLoadOp::kClear;
    passDesc.colourAttachments[2].format          = kDeferredBufferCFormat;
    passDesc.colourAttachments[2].loadOp          = GPURenderLoadOp::kClear;
    passDesc.depthStencilAttachment               = GPURenderAttachmentDesc();
    passDesc.depthStencilAttachment.format        = kDepthBufferFormat;
    passDesc.depthStencilAttachment.loadOp        = GPURenderLoadOp::kClear;
    passDesc.depthStencilAttachment.stencilLoadOp = GPURenderLoadOp::kDontCare;

    this->gBufferPass = g_gpuManager->createRenderPass(std::move(passDesc));

    /* Create the lighting pass. */
    passDesc.colourAttachments.resize(1);
    passDesc.colourAttachments[0].format          = kLinearLDRColourBufferFormat;
    passDesc.colourAttachments[0].loadOp          = GPURenderLoadOp::kClear;
    passDesc.depthStencilAttachment.format        = kDepthBufferFormat;
    passDesc.depthStencilAttachment.loadOp        = GPURenderLoadOp::kLoad;
    passDesc.depthStencilAttachment.stencilLoadOp = GPURenderLoadOp::kDontCare;

    this->lightPass = g_gpuManager->createRenderPass(std::move(passDesc));

    /* Create the basic material pass. */
    passDesc.colourAttachments.resize(1);
    passDesc.colourAttachments[0].format          = kLinearLDRColourBufferFormat;
    passDesc.colourAttachments[0].loadOp          = GPURenderLoadOp::kLoad;
    passDesc.depthStencilAttachment.format        = kDepthBufferFormat;
    passDesc.depthStencilAttachment.loadOp        = GPURenderLoadOp::kLoad;
    passDesc.depthStencilAttachment.stencilLoadOp = GPURenderLoadOp::kDontCare;

    this->basicPass = g_gpuManager->createRenderPass(std::move(passDesc));
}

/** Initialise the pipeline. */
DeferredRenderPipeline::DeferredRenderPipeline() :
    shadowMapResolution(kDefaultShadowMapResolution)
{
    /* Ensure that global resources are initialised. */
    m_resources.init();

    #if ORION_BUILD_DEBUG
        this->debugDrawLights = false;
    #endif
}

/** Destroy the pipeline. */
DeferredRenderPipeline::~DeferredRenderPipeline() {}

/** Render a world.
 * @param world         World to render.
 * @param view          View to render from.
 * @param target        Render target. */
void DeferredRenderPipeline::render(const RenderWorld &world, RenderView &view, RenderTarget &target) const {
    Context context(world, view, target);

    allocateResources(context);

    /* Get lists of visible entities and lights. */
    context.cull(context.cullResults);

    prepareLights(context);
    prepareEntities(context);

    renderShadowMaps(context);
    renderDeferred(context);
    renderBasic(context);

    /* Perform post-processing effects and output the image to the real render
     * target. */
    renderPostEffects(context, context.colourBuffer, ImageType::kLinearLDR);

    /* Render debug primitives. */
    renderDebug(context);
}

/** Allocate rendering resources.
 * @param context       Rendering context. */
void DeferredRenderPipeline::allocateResources(Context &context) const {
    context.renderArea = IntRect(glm::ivec2(0, 0), context.view().viewport().size());

    auto textureDesc = GPUTextureDesc().
        setType   (GPUTexture::kTexture2D).
        setWidth  (context.renderArea.width).
        setHeight (context.renderArea.height).
        setMips   (1).
        setFlags  (GPUTexture::kRenderTarget);

    /* Allocate the main output textures. */
    textureDesc.format      = kLinearLDRColourBufferFormat;
    context.colourBuffer    = g_renderTargetPool->allocate(textureDesc);
    textureDesc.format      = kDepthBufferFormat;
    context.depthBuffer     = g_renderTargetPool->allocate(textureDesc);

    /* Allocate the G-Buffer textures. */
    textureDesc.format      = kDeferredBufferAFormat;
    context.deferredBufferA = g_renderTargetPool->allocate(textureDesc);
    textureDesc.format      = kDeferredBufferBFormat;
    context.deferredBufferB = g_renderTargetPool->allocate(textureDesc);
    textureDesc.format      = kDeferredBufferCFormat;
    context.deferredBufferC = g_renderTargetPool->allocate(textureDesc);
    textureDesc.format      = kDeferredBufferDFormat;
    context.deferredBufferD = g_renderTargetPool->allocate(textureDesc);

    /* Create a material to refer to the render targets. */
    context.lightMaterial = new Material(m_resources->lightShader);

    GPUSamplerStatePtr sampler = g_gpuManager->getSamplerState();
    context.lightMaterial->setGPUTexture("deferredBufferA", context.deferredBufferA, sampler);
    context.lightMaterial->setGPUTexture("deferredBufferB", context.deferredBufferB, sampler);
    context.lightMaterial->setGPUTexture("deferredBufferC", context.deferredBufferC, sampler);
    context.lightMaterial->setGPUTexture("deferredBufferD", context.deferredBufferD, sampler);
}

/** Prepare light state.
 * @param context       Rendering context. */
void DeferredRenderPipeline::prepareLights(Context &context) const {
    context.lights.reserve(context.cullResults.lights.size());

    for (RenderLight *renderLight : context.cullResults.lights) {
        /* Debug light rendering. */
        #if ORION_BUILD_DEBUG
            if (this->debugDrawLights) {
                switch (renderLight->type()) {
                    case RenderLight::kPointLight:
                    case RenderLight::kSpotLight:
                    {
                        glm::vec3 position = renderLight->position();
                        BoundingBox boundingBox(
                            position - glm::vec3(0.2f, 0.2f, 0.2f),
                            position + glm::vec3(0.2f, 0.2f, 0.2f));

                        glm::vec4 colour = glm::vec4(renderLight->colour(), 1.0f);
                        g_debugManager->draw(boundingBox, colour, true);

                        if (renderLight->type() == RenderLight::kSpotLight) {
                            g_debugManager->drawLine(position,
                                                     position + renderLight->direction(),
                                                     colour,
                                                     true);
                        }

                        break;
                    }

                    default:
                        break;
                }
            }
        #endif

        context.lights.emplace_back();
        Light &light = context.lights.back();

        light.renderLight = renderLight;

        /* Flush resource updates. */
        light.resources = renderLight->getResources();

        if (renderLight->castsShadows()) {
            allocateShadowMap(light);

            /* Update the shadow map resource binding. */
            GPUSamplerStatePtr sampler = g_gpuManager->getSamplerState(GPUSamplerStateDesc().
                setFilterMode    (SamplerFilterMode::kBilinear).
                setCompareEnable (true).
                setCompareFunc   (ComparisonFunc::kLess));
            light.resources->bindTexture(ResourceSlots::kShadowMap, light.shadowMap, sampler);

            /* Now find all shadow casting entities which are affected by this
             * light. */
            const unsigned numShadowViews = renderLight->numShadowViews();
            for (unsigned i = 0; i < numShadowViews; i++) {
                RenderView &shadowView = renderLight->shadowView(i);

                /* Update the viewport for the current shadow map resolution. */
                IntRect viewport(0, 0, this->shadowMapResolution, this->shadowMapResolution);
                shadowView.setViewport(viewport);

                /* TODO: Could maybe exclude non-shadow casting entities during
                 * the cull process? */
                auto &cullResults = light.shadowMapCullResults[i];
                context.world().cull(shadowView, cullResults, 0);

                for (RenderEntity *entity : cullResults.entities) {
                    if (entity->castsShadow()) {
                        Shader *shader = entity->material()->shader();

                        if (shader->numPasses(kShadowCasterPassType) > 0) {
                            light.shadowMapDrawLists[i].add(entity, kShadowCasterPassType);
                        } else {
                            logWarning("Shader for shadow casting entity '%s' lacks shadow caster pass",
                                       entity->name.c_str());
                        }
                    }
                }
            }
        }
    }
}

/** Allocate a shadow map for a light.
 * @param light         Light to allocate for. */
void DeferredRenderPipeline::allocateShadowMap(Light &light) const {
    GPUTexture::Type type;
    switch (light.renderLight->type()) {
        case RenderLight::kSpotLight:
            type = GPUTexture::kTexture2D;
            break;
        case RenderLight::kPointLight:
            type = GPUTexture::kTextureCube;
            break;
        default:
            fatal("TODO");
            break;
    }

    auto desc = GPUTextureDesc().
        setType   (type).
        setWidth  (this->shadowMapResolution).
        setHeight (this->shadowMapResolution).
        setMips   (1).
        setFlags  (GPUTexture::kRenderTarget).
        setFormat (kShadowMapFormat);

    light.shadowMap = g_renderTargetPool->allocate(desc);
}

/** Prepare entity state.
 * @param context       Rendering context. */
void DeferredRenderPipeline::prepareEntities(Context &context) const {
    for (RenderEntity *entity : context.cullResults.entities) {
        Shader *shader = entity->material()->shader();

        if (shader->numPasses(kDeferredPassType) > 0) {
            context.deferredDrawList.add(entity, kDeferredPassType);
        } else if (shader->numPasses(Pass::kBasicType) > 0) {
            context.basicDrawList.add(entity, Pass::kBasicType);
        } else {
            logWarning("Don't know how to draw entity '%s'", entity->name.c_str());
        }
    }
}

/** Render shadow maps.
 * @param context       Rendering context. */
void DeferredRenderPipeline::renderShadowMaps(Context &context) const {
    GPU_DEBUG_GROUP("Shadow Maps");

    for (Light &light : context.lights) {
        if (!light.shadowMap)
            continue;

        RenderLight *renderLight = light.renderLight;

        GPU_DEBUG_GROUP("Light '%s'", renderLight->name.c_str());

        const unsigned numShadowViews = renderLight->numShadowViews();
        for (unsigned i = 0; i < numShadowViews; i++) {
            GPU_DEBUG_GROUP("View %u", i);

            RenderView &shadowView = renderLight->shadowView(i);

            GPURenderPassInstanceDesc passDesc(m_resources->shadowMapPass);
            passDesc.targets.depthStencil.texture = light.shadowMap;
            passDesc.targets.depthStencil.layer   = i;
            passDesc.clearDepth                   = 1.0;
            passDesc.renderArea                   = shadowView.viewport();

            GPUCommandList *cmdList = g_gpuManager->beginRenderPass(passDesc);

            /* Bind resources. */
            cmdList->bindResourceSet(ResourceSets::kLightResources, light.resources);
            cmdList->bindResourceSet(ResourceSets::kViewResources, shadowView.getResources());

            /* Render the shadow map. Default state is what we want here:
             * blending disabled, depth test/write enabled. */
            light.shadowMapDrawLists[i].draw(cmdList, ShaderKeywordSet());

            g_gpuManager->submitRenderPass(cmdList);
        }
    }
}

/** Perform deferred rendering.
 * @param context       Rendering context. */
void DeferredRenderPipeline::renderDeferred(Context &context) const {
    GPU_DEBUG_GROUP("Deferred Rendering");

    renderDeferredGBuffer(context);
    renderDeferredLights(context);
}

/** Render the G-Buffer.
 * @param context       Rendering context. */
void DeferredRenderPipeline::renderDeferredGBuffer(Context &context) const {
    GPU_DEBUG_GROUP("G-Buffer Pass");

    GPURenderPassInstanceDesc passDesc(m_resources->gBufferPass);
    passDesc.targets.colour[0].texture    = context.deferredBufferA;
    passDesc.targets.colour[1].texture    = context.deferredBufferB;
    passDesc.targets.colour[2].texture    = context.deferredBufferC;
    passDesc.targets.depthStencil.texture = context.depthBuffer;
    passDesc.clearColours[0]              = glm::vec4(0.0, 0.0, 0.0, 0.0);
    passDesc.clearColours[1]              = glm::vec4(0.0, 0.0, 0.0, 0.0);
    passDesc.clearColours[2]              = glm::vec4(0.0, 0.0, 0.0, 0.0);
    passDesc.clearDepth                   = 1.0;
    passDesc.clearStencil                 = 0;
    passDesc.renderArea                   = context.renderArea;

    GPUCommandList *cmdList = g_gpuManager->beginRenderPass(passDesc);

    /* Bind view resources. */
    cmdList->bindResourceSet(ResourceSets::kViewResources, context.view().getResources());

    /* Render everything to the G-Buffer. Default state is what we want,
     * blending disabled, depth test/write enabled. */
    context.deferredDrawList.draw(cmdList, ShaderKeywordSet());

    g_gpuManager->submitRenderPass(cmdList);

    /* Make a copy of the depth buffer. We need to do this as we want to
     * keep the same depth buffer while rendering light volumes, but the
     * light shaders need to read the depth buffer. */
    g_gpuManager->blit(GPUTextureImageRef(context.depthBuffer),
                       GPUTextureImageRef(context.deferredBufferD),
                       context.renderArea.pos(),
                       context.renderArea.pos(),
                       context.renderArea.size());
}

/** Perform deferred light rendering.
 * @param context       Rendering context. */
void DeferredRenderPipeline::renderDeferredLights(Context &context) const {
    GPU_DEBUG_GROUP("Light Pass");

    /* Begin the light pass on the primary render target. */
    GPURenderPassInstanceDesc passDesc(m_resources->lightPass);
    passDesc.targets.colour[0].texture    = context.colourBuffer;
    passDesc.targets.depthStencil.texture = context.depthBuffer;
    passDesc.clearColours[0]              = glm::vec4(0.0, 0.0, 0.0, 1.0);
    passDesc.renderArea                   = context.renderArea;

    GPUCommandList *cmdList = g_gpuManager->beginRenderPass(passDesc);

    /* Set up state for the light material. */
    context.lightMaterial->setDrawState(cmdList);

    /* Bind view resources. */
    cmdList->bindResourceSet(ResourceSets::kViewResources, context.view().getResources());

    /* Light volumes should be rendered with additive blending. */
    cmdList->setBlendState(GPUBlendStateDesc().
        setFunc         (BlendFunc::kAdd).
        setSourceFactor (BlendFactor::kOne).
        setDestFactor   (BlendFactor::kOne));

    for (Light &light : context.lights) {
        GPU_CMD_DEBUG_GROUP(cmdList, "Light '%s'", light.renderLight->name.c_str());

        /* Set up rasterizer/depth testing state. No depth writes here, the
         * light volumes should not affect our depth buffer. */
        switch (light.renderLight->type()) {
            case RenderLight::kAmbientLight:
            case RenderLight::kDirectionalLight:
                /* These are rendered as full-screen quads and should have
                 * their front faces unconditionally rendered. */
                cmdList->setDepthStencilState(GPUDepthStencilStateDesc().
                    setDepthFunc  (ComparisonFunc::kAlways).
                    setDepthWrite (false));

                cmdList->setRasterizerState();
                break;

            default:
                /* For others we want to render their back faces, so that
                 * they will still be rendered even if the view is inside
                 * the light volume. Test for depth greater than or equal to
                 * the back face of the light volume so that only pixels in
                 * front of it are touched. Additionally, enable depth
                 * clamping so that the light volume is not clipped. */
                cmdList->setDepthStencilState(GPUDepthStencilStateDesc().
                    setDepthFunc  (ComparisonFunc::kGreaterOrEqual).
                    setDepthWrite (false));

                cmdList->setRasterizerState(GPURasterizerStateDesc().
                    setCullMode   (CullMode::kFront).
                    setDepthClamp (true));

                break;

        }

        /* Set up the appropriate pass from the light shader. */
        ShaderKeywordSet variation;
        variation.insert(kLightVariations[light.renderLight->type()]);
        if (light.renderLight->castsShadows())
            variation.insert(kShadowVariation);

        const Pass *pass = m_resources->lightShader->getPass(kDeferredLightPassType, 0);
        pass->setDrawState(cmdList, variation);

        /* Set light resources. */
        cmdList->bindResourceSet(ResourceSets::kLightResources, light.resources);

        /* Draw the light volume. */
        Geometry geometry = light.renderLight->volumeGeometry();
        cmdList->draw(geometry.primitiveType, geometry.vertices, geometry.indices);
    }

    g_gpuManager->submitRenderPass(cmdList);
}

/** Render basic materials.
 * @param context       Rendering context. */
void DeferredRenderPipeline::renderBasic(Context &context) const {
    GPU_DEBUG_GROUP("Basic");

    GPURenderPassInstanceDesc passDesc(m_resources->basicPass);
    passDesc.targets.colour[0].texture    = context.colourBuffer;
    passDesc.targets.depthStencil.texture = context.depthBuffer;
    passDesc.renderArea                   = context.renderArea;

    GPUCommandList *cmdList = g_gpuManager->beginRenderPass(passDesc);

    /* Bind view resources. */
    cmdList->bindResourceSet(ResourceSets::kViewResources, context.view().getResources());

    context.basicDrawList.draw(cmdList, ShaderKeywordSet());

    g_gpuManager->submitRenderPass(cmdList);
}
