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

#include "render/render_pipeline.h"

#include "render_core/material.h"
#include "render_core/render_target_pool.h"

/** Screen buffer pixel formats. */
static const PixelFormat kColourBufferFormat = PixelFormat::kR8G8B8A8;
static const PixelFormat kDepthBufferFormat = PixelFormat::kDepth24Stencil8;

/**
 * G-Buffer pixel formats. The buffer layout is as follows:
 *
 *     | Format      | R          | G          | B          | A
 *  ---|-------------|------------|------------|------------|------------
 *   A | R10G10B10A2 | Normal.x   | Normal.y   | Normal.z   | -
 *  ---|-------------|------------|------------|------------|------------
 *   B | R8G8B8A8    | Diffuse.r  | Diffuse.g  | Diffuse.b  | -
 *  ---|-------------|------------|------------|------------|------------
 *   C | R8G8B8A8    | Specular.r | Specular.g | Specular.b | 1/Shininess
 *  ---|-------------|------------|------------|------------|------------
 *   D | D24S8       | Depth      | -          | -          | -
 *
 * These are all unsigned normalized textures, therefore the normals are scaled
 * to fit into the [0, 1] range, and the shininess is stored as its reciprocal.
 * Position is reconstructed from the depth buffer.
 */
static const PixelFormat kDeferredBufferAFormat = PixelFormat::kR10G10B10A2;
static const PixelFormat kDeferredBufferBFormat = PixelFormat::kR8G8B8A8;
static const PixelFormat kDeferredBufferCFormat = PixelFormat::kR8G8B8A8;
static const PixelFormat kDeferredBufferDFormat = PixelFormat::kDepth24Stencil8;

/** Rendering pipeline implementing deferred rendering. */
class DeferredRenderPipeline final : public RenderPipeline {
public:
    CLASS();

    /** Rendering context. */
    struct Context : RenderContext {
        /** Main output textures. */
        RenderTargetPool::Handle colourBuffer;
        RenderTargetPool::Handle depthBuffer;

        /** G-Buffer textures. */
        RenderTargetPool::Handle deferredBufferA;
        RenderTargetPool::Handle deferredBufferB;
        RenderTargetPool::Handle deferredBufferC;
        RenderTargetPool::Handle deferredBufferD;
    public:
        Context(const RenderWorld &world, RenderView &view, RenderTarget &target);
    };

    DeferredRenderPipeline();
    ~DeferredRenderPipeline();

    void render(const RenderWorld &world, RenderView &view, RenderTarget &target) override;
private:
    /** Global resources for the pipeline. */
    struct Resources {
        MaterialPtr lightMaterial;      /**< Deferred light material. */
    public:
        Resources();
    };

    static GlobalResource<Resources> m_resources;
};
