/*
 * Copyright (C) 2016 Alex Smith
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
 * @brief               Vulkan stub functions.
 */

#include "vulkan.h"

#define vkStub() fatal("Vulkan function '%s' not implemented", __func__)

/** Begin a render pass.
 * @param desc          Descriptor for the render pass instance. */
void VulkanGPUManager::beginRenderPass(const GPURenderPassInstanceDesc &desc) {
    // TODO: Need to transition images. Could have shader read as default state,
    // transition to right attachment layout at beginning and end.
    vkStub();
}

/** End the current render pass. */
void VulkanGPUManager::endRenderPass() {
    vkStub();
}

/** Bind a pipeline for rendering.
 * @param pipeline      Pipeline to use. */
void VulkanGPUManager::bindPipeline(GPUPipeline *pipeline) {
    vkStub();
}

/** Bind a resource set.
 * @param index         Resource set index to bind to.
 * @param resources     Resource set to bind. */
void VulkanGPUManager::bindResourceSet(unsigned index, GPUResourceSet *resources) {
    vkStub();
}

/** Set the blend state.
 * @param state         Blend state to set. */
void VulkanGPUManager::setBlendState(GPUBlendState *state) {
    vkStub();
}

/** Set the depth/stencil state.
 * @param state         Depth/stencil state to set. */
void VulkanGPUManager::setDepthStencilState(GPUDepthStencilState *state) {
    vkStub();
}

/** Set the rasterizer state.
 * @param state         Rasterizer state to set. */
void VulkanGPUManager::setRasterizerState(GPURasterizerState *state) {
    vkStub();
}

/** Set the viewport.
 * @param viewport      Viewport rectangle in pixels. */
void VulkanGPUManager::setViewport(const IntRect &viewport) {
    vkStub();
}

/** Set the scissor test parameters.
 * @param enable        Whether to enable scissor testing.
 * @param scissor       Scissor rectangle. */
void VulkanGPUManager::setScissor(bool enable, const IntRect &scissor) {
    vkStub();
}

/** Draw primitives.
 * @param type          Primitive type to render.
 * @param vertices      Vertex data to use.
 * @param indices       Index data to use (can be null). */
void VulkanGPUManager::draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) {
    vkStub();
}
