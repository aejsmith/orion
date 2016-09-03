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

/** Create a sampler state object.
 * @param desc          Descriptor for sampler state.
 * @return              Pointer to created sampler state object. */
GPUSamplerStatePtr VulkanGPUManager::createSamplerState(const GPUSamplerStateDesc &desc) {
    vkStub();
}

/** Create a texture.
 * @see                 GPUTexture::GPUTexture().
 * @param desc          Descriptor containing texture parameters.
 * @return              Pointer to created texture. */
GPUTexturePtr VulkanGPUManager::createTexture(const GPUTextureDesc &desc) {
    vkStub();
}

/** Create a texture view.
 * @param image         Image to create the view for.
 * @return              Pointer to created texture view. */
GPUTexturePtr VulkanGPUManager::createTextureView(const GPUTextureImageRef &image) {
    vkStub();
}

/** Create a resource set.
 * @param layout        Layout for the resource set.
 * @return              Pointer to created resource set. */
GPUResourceSetPtr VulkanGPUManager::createResourceSet(GPUResourceSetLayout *layout) {
    vkStub();
}

/** Begin a render pass.
 * @param desc          Descriptor for the render pass instance. */
void VulkanGPUManager::beginRenderPass(const GPURenderPassInstanceDesc &desc) {
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

/** Copy pixels from one texture to another.
 * @param source        Source texture image reference.
 * @param dest          Destination texture image reference.
 * @param sourcePos     Position in source texture to copy from.
 * @param destPos       Position in destination texture to copy to.
 * @param size          Size of area to copy. */
void VulkanGPUManager::blit(
    const GPUTextureImageRef &source,
    const GPUTextureImageRef &dest,
    glm::ivec2 sourcePos,
    glm::ivec2 destPos,
    glm::ivec2 size)
{
    vkStub();
}

/** Draw primitives.
 * @param type          Primitive type to render.
 * @param vertices      Vertex data to use.
 * @param indices       Index data to use (can be null). */
void VulkanGPUManager::draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) {
    vkStub();
}

#ifdef ORION_BUILD_DEBUG

/** Begin a debug group.
 * @param str           Group string. */
void VulkanGPUManager::beginDebugGroup(const std::string &str) {
    vkStub();
}

/** End the current debug group. */
void VulkanGPUManager::endDebugGroup() {
    vkStub();
}

#endif
