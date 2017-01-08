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

#include "render/deferred_render_pipeline.h"

/* Register the deferred pass type. */
DEFINE_PASS_TYPE("Deferred", {});

/* Register the shadow caster pass type. */
DEFINE_PASS_TYPE("ShadowCaster", {});

/* Register the deferred light pass type. Needs a variation per light type. */
DEFINE_PASS_TYPE("DeferredLight", {
    {"AMBIENT_LIGHT"},
    {"DIRECTIONAL_LIGHT"},
    {"DIRECTIONAL_LIGHT", "SHADOW"},
    {"POINT_LIGHT"},
    {"POINT_LIGHT", "SHADOW"},
    {"SPOT_LIGHT"},
    {"SPOT_LIGHT", "SHADOW"}
});

/** Global resources for the deferred pipeline. */
GlobalResource<DeferredRenderPipeline::Resources> DeferredRenderPipeline::m_resources;

/** Initialise global deferred rendering resources. */
DeferredRenderPipeline::Resources::Resources() {
    /* Load the light material. */
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/deferred_light");
    this->lightMaterial = new Material(shader);
}

/** Initialise the pipeline. */
DeferredRenderPipeline::DeferredRenderPipeline() {
    /* Ensure that global resources are initialised. */
    m_resources.init();
}

/** Destroy the pipeline. */
DeferredRenderPipeline::~DeferredRenderPipeline() {}

/** Render a world.
 * @param world         World to render.
 * @param view          View to render from.
 * @param target        Render target. */
void DeferredRenderPipeline::render(const RenderWorld *world, RenderView *view, RenderTarget *target) {
    RenderContext context(world, view, target);
}
