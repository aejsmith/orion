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

#pragma once

#include "engine/global_resource.h"

#include "render/draw_list.h"
#include "render/render_light.h"
#include "render/render_pipeline.h"
#include "render/render_world.h"

#include "render_core/material.h"
#include "render_core/render_target_pool.h"

/**
 * G-Buffer pixel formats. The buffer layout is as follows:
 *
 *     | Format            | R          | G          | B          | A
 *  ---|-------------------|------------|------------|------------|-----------
 *   A | R10G10B10A2       | Normal.x   | Normal.y   | Normal.z   | -
 *  ---|-------------------|------------|------------|------------|-----------
 *   B | FloatR16G16B16A16 | Diffuse.r  | Diffuse.g  | Diffuse.b  | -
 *  ---|-------------------|------------|------------|------------|-----------
 *   C | FloatR16G16B16A16 | Specular.r | Specular.g | Specular.b | Shininess
 *  ---|-------------------|------------|------------|------------|-----------
 *   D | D32               | Depth      | -          | -          | -
 *
 * The normal buffer is an unsigned normalized format, therefore the normals
 * are scaled to fit into the [0, 1] range. Position is reconstructed from the
 * depth buffer.
 */
static const PixelFormat kDeferredBufferAFormat = PixelFormat::kR10G10B10A2;
static const PixelFormat kDeferredBufferBFormat = PixelFormat::kFloatR16G16B16A16;
static const PixelFormat kDeferredBufferCFormat = PixelFormat::kFloatR16G16B16A16;
static const PixelFormat kDeferredBufferDFormat = PixelFormat::kDepth32;

/** Shadow map format. TODO: Investigate lowering this to D16. */
static const PixelFormat kShadowMapFormat       = PixelFormat::kDepth32;

/** Rendering pipeline implementing deferred rendering. */
class DeferredRenderPipeline final : public RenderPipeline {
public:
    CLASS();

    DeferredRenderPipeline();
    ~DeferredRenderPipeline();

    /** Resolution to use for shadow maps. */
    PROPERTY() uint16_t shadowMapResolution;

    #if ORION_BUILD_DEBUG

    /** Debug options. */
    PROPERTY("transient": true) bool debugDrawLights;

    #endif

    void render(const RenderWorld &world, RenderView &view, RenderTarget &target) const override;
private:
    /** Global resources for the pipeline. */
    struct Resources {
        /** Deferred light shader. */
        ShaderPtr lightShader;

        /** Render passes. */
        GPURenderPassPtr shadowMapPass;         /**< Shadow map pass. */
        GPURenderPassPtr gBufferPass;           /**< G-Buffer render pass. */
        GPURenderPassPtr lightPass;             /**< Deferred light render pass. */
        GPURenderPassPtr basicPass;             /**< Basic render pass. */
    public:
        Resources();
    };

    /** Per-light state. */
    struct Light {
        RenderLight *renderLight;               /**< Light object. */
        GPUResourceSet *resources;              /**< Resources for the light. */
        RenderTargetPool::Handle shadowMap;     /**< Shadow map for the light. */

        /** Shadow map culling results per view. */
        RenderWorld::CullResults shadowMapCullResults[RenderLight::kMaxShadowViews];

        /** Shadow map draw lists per view. */
        DrawList shadowMapDrawLists[RenderLight::kMaxShadowViews];
    };

    /** Rendering context. */
    struct Context : RenderContext {
        /** Rendering area. */
        IntRect renderArea;

        /** Main output textures. */
        RenderTargetPool::Handle colourBuffer;
        RenderTargetPool::Handle depthBuffer;

        /** G-Buffer textures. */
        RenderTargetPool::Handle deferredBufferA;
        RenderTargetPool::Handle deferredBufferB;
        RenderTargetPool::Handle deferredBufferC;
        RenderTargetPool::Handle deferredBufferD;

        /** Light material. */
        MaterialPtr lightMaterial;

        /** Culling results. */
        RenderWorld::CullResults cullResults;

        /** Per-light state. */
        std::vector<Light> lights;

        /** List of draw calls for entities with deferred passes. */
        DrawList deferredDrawList;

        /** List of draw calls for entities with basic passes. */
        DrawList basicDrawList;
    public:
        using RenderContext::RenderContext;
    };

    void allocateResources(Context &context) const;

    void prepareLights(Context &context) const;
    void allocateShadowMap(Light &light) const;

    void prepareEntities(Context &context) const;

    void renderShadowMaps(Context &context) const;
    void renderDeferred(Context &context) const;
    void renderDeferredGBuffer(Context &context) const;
    void renderDeferredLights(Context &context) const;
    void renderBasic(Context &context) const;

    static GlobalResource<Resources> m_resources;
};
