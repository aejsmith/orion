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
 * @brief               Render target class.
 */

#include "engine/engine.h"
#include "engine/render_target.h"

#include "gpu/gpu_manager.h"

/**
 * Initialize the layer.
 *
 * Initializes the layer. The layer is initially not associated with a render
 * target, derived classes must set a default target themselves, and register
 * themselves with the target.
 *
 * @param priority      Rendering priority.
 */
RenderLayer::RenderLayer(unsigned priority) :
    m_renderTarget(nullptr),
    m_viewport(0.0f, 0.0f, 1.0f, 1.0f),
    m_pixelViewport(0, 0, 0, 0),
    m_priority(priority),
    m_registered(false)
{}

/** Destroy the layer. */
RenderLayer::~RenderLayer() {
    if (m_registered)
        unregisterRenderLayer();
}

/** Set the render target.
 * @param target        New render target. */
void RenderLayer::setRenderTarget(RenderTarget *target) {
    /* Free any render pass we have. We will recreate it for the new target the
     * first time beginRenderPass() is called. */
    if (m_renderPass)
        m_renderPass.reset();

    if (m_registered)
        m_renderTarget->removeLayer(this);

    m_renderTarget = target;

    /* Recalculate pixel viewport. */
    setViewport(m_viewport);

    if (m_registered)
        m_renderTarget->addLayer(this);
}

/**
 * Set the viewport.
 *
 * Sets the viewport rectangle. Coordinates are normalized, range from (0, 0)
 * in the bottom left corner to (1, 1) in the top right corner. The actual
 * viewport rectangle is calculated automatically based on the render target's
 * dimensions.
 *
 * @param viewport      Normalized viewport rectangle.
 */
void RenderLayer::setViewport(const Rect &viewport) {
    m_viewport = viewport;

    uint32_t targetWidth = m_renderTarget->width();
    uint32_t targetHeight = m_renderTarget->height();

    m_pixelViewport = IntRect(
        m_viewport.x * static_cast<float>(targetWidth),
        m_viewport.y * static_cast<float>(targetHeight),
        m_viewport.width * static_cast<float>(targetWidth),
        m_viewport.height * static_cast<float>(targetHeight));

    viewportChanged();
}

/**
 * Set the rendering priority.
 *
 * Set the layer rendering priority. Each layer on a render target has a
 * priority value which defines the order in which layers on the target are
 * rendered. Layers with higher priority values will appear on top of those with
 * lower values.
 *
 * @param priority      New rendering priority.
 */
void RenderLayer::setRenderPriority(unsigned priority) {
    /* If we are registered we have to re-register in the correct place in
     * the target's layer list. */
    if (m_registered)
        m_renderTarget->removeLayer(this);

    m_priority = priority;

    if (m_registered)
        m_renderTarget->addLayer(this);
}

/** Register the layer with its render target. */
void RenderLayer::registerRenderLayer() {
    check(m_renderTarget);
    check(!m_registered);

    m_renderTarget->addLayer(this);
    m_registered = true;
}

/** Unregister the layer from its render target. */
void RenderLayer::unregisterRenderLayer() {
    check(m_registered);

    m_renderTarget->removeLayer(this);
    m_registered = false;
}

/**
 * Begin a render pass instance for the layer.
 *
 * This function begins a new render pass to render to the render target for
 * this layer. The render pass will only have a colour target, no depth/stencil
 * target, therefore this pass should be used with depth/stencil testing and
 * writes disabled. GPUManager::endRenderPass() should be called at the end of
 * the pass.
 *
 * @param loadOp        Load operation for the colour target.
 * @param clearColour   If loadOp is GPURenderLoadOp::kClear, the colour to
 *                      clear to.
 */
void RenderLayer::beginLayerRenderPass(GPURenderLoadOp loadOp, const glm::vec4 &clearColour) {
    if (!m_renderPass) {
        /* Need to create a new render pass. */
        GPURenderPassDesc passDesc(1);
        passDesc.colourAttachments[0].format = m_renderTarget->format();
        passDesc.colourAttachments[0].loadOp = loadOp;
        m_renderPass = g_gpuManager->createRenderPass(std::move(passDesc));
    }

    check(m_renderPass->desc().colourAttachments[0].loadOp == loadOp);

    GPURenderPassInstanceDesc instanceDesc(m_renderPass);
    m_renderTarget->getRenderTargetDesc(instanceDesc.targets);
    instanceDesc.clearColours[0] = clearColour;
    instanceDesc.renderArea = m_pixelViewport;
    g_gpuManager->beginRenderPass(instanceDesc);
}

/** Initialize the render target.
 * @param width         Width of the render target.
 * @param height        Height of the render target.
 * @param format        Pixel format of the render target.
 * @param priority      Rendering priority. */
RenderTarget::RenderTarget(uint32_t width, uint32_t height, PixelFormat format, unsigned priority) :
    m_width(width),
    m_height(height),
    m_format(format),
    m_priority(priority)
{}

/** Destroy the render target. */
RenderTarget::~RenderTarget() {
    checkMsg(m_layers.empty(), "Destroying RenderTarget with active layers");
}

/** Add a layer to the render target.
 * @param layer         Layer to add. */
void RenderTarget::addLayer(RenderLayer *layer) {
    bool wasEmpty = m_layers.empty();

    /* List is sorted by priority. */
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        RenderLayer *exist = *it;
        if (layer->renderPriority() < exist->renderPriority()) {
            m_layers.insert(it, layer);
            return;
        }
    }

    /* Insertion point not found, add at end. */
    m_layers.push_back(layer);

    if (wasEmpty)
        g_engine->addRenderTarget(this);
}

/** Remove a layer from the render target.
 * @param layer         Layer to remove. */
void RenderTarget::removeLayer(RenderLayer *layer) {
    m_layers.remove(layer);

    if (m_layers.empty())
        g_engine->removeRenderTarget(this);
}

/** Render the render target. */
void RenderTarget::render() {
    GPU_DEBUG_GROUP("RenderTarget '%s'", renderTargetName().c_str());

    /* Render all our layers. */
    for (RenderLayer *layer : m_layers) {
        GPU_DEBUG_GROUP("%s", layer->renderLayerName().c_str());
        layer->render();
    }
}
