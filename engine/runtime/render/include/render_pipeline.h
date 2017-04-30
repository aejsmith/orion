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
 * @brief               Rendering pipeline class.
 */

#pragma once

#include "engine/object.h"

#include "render/render_context.h"

#include "render_core/render_target_pool.h"

class PostEffect;

/** Screen buffer pixel formats. */
static const PixelFormat kColourBufferFormat = PixelFormat::kR8G8B8A8;
static const PixelFormat kDepthBufferFormat  = PixelFormat::kDepth32;

/**
 * Rendering pipeline base class.
 *
 * This class is the base for a rendering pipeline, which implements the process
 * for rendering the world.
 */
class RenderPipeline : public Object {
public:
    CLASS();

    ~RenderPipeline();

    /** Global resources for all pipelines. */
    struct BaseResources {
        /** Render passes. */
        GPURenderPassPtr postEffectPass;        /**< Post-processing effect pass. */
        GPURenderPassPtr debugPass;             /**< Debug rendering pass. */
    public:
        BaseResources();
    };

    static const BaseResources &resources();

    /**
     * Render a world.
     *
     * Renders the given world from a view to a render target. This is expected
     * to set up a RenderContext (or derived class) based on these parameters,
     * and then use methods on that to render the world.
     *
     * @param world         World to render.
     * @param view          View to render from.
     * @param target        Render target.
     */
    virtual void render(const RenderWorld &world, RenderView &view, RenderTarget &target) const = 0;

    void addPostEffect(ObjectPtr<PostEffect> effect);
protected:
    RenderPipeline();

    void serialise(Serialiser &serialiser) const override;
    void deserialise(Serialiser &serialiser) override;

    RenderTargetPool::Handle renderPostEffects(const RenderTargetPool::Handle &input) const;

    void renderDebug(RenderContext &context, const RenderTargetPool::Handle &texture) const;
private:
    /** List of post processing effects. */
    std::list<ObjectPtr<PostEffect>> m_postEffects;
};
