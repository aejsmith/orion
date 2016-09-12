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
 * @brief               Vulkan rendering commands.
 */

#include "command_buffer.h"
#include "frame.h"

/** Bind a pipeline for rendering.
 * @param pipeline      Pipeline to use. */
void VulkanGPUManager::bindPipeline(GPUPipeline *pipeline) {
    currentFrame().pipeline = static_cast<VulkanPipeline *>(pipeline);
}

/** Bind a resource set.
 * @param index         Resource set index to bind to.
 * @param resources     Resource set to bind. */
void VulkanGPUManager::bindResourceSet(unsigned index, GPUResourceSet *resources) {
    check(index < currentFrame().resourceSets.size());
    currentFrame().resourceSets[index] = static_cast<VulkanResourceSet *>(resources);
}

/** Set the blend state.
 * @param state         Blend state to set. */
void VulkanGPUManager::setBlendState(GPUBlendState *state) {
    currentFrame().blendState = state;
}

/** Set the depth/stencil state.
 * @param state         Depth/stencil state to set. */
void VulkanGPUManager::setDepthStencilState(GPUDepthStencilState *state) {
    currentFrame().depthStencilState = state;
}

/** Set the rasterizer state.
 * @param state         Rasterizer state to set. */
void VulkanGPUManager::setRasterizerState(GPURasterizerState *state) {
    currentFrame().rasterizerState = state;
}

/** Set the viewport.
 * @param viewport      Viewport rectangle in pixels. */
void VulkanGPUManager::setViewport(const IntRect &viewport) {
    currentFrame().viewport = viewport;
}

/** Set the scissor test parameters.
 * @param enable        Whether to enable scissor testing.
 * @param scissor       Scissor rectangle. */
void VulkanGPUManager::setScissor(bool enable, const IntRect &scissor) {
    currentFrame().scissorEnabled = enable;
    currentFrame().scissor = scissor;
}

#ifdef ORION_BUILD_DEBUG

/** Begin a debug group.
 * @param str           Group string. */
void VulkanGPUManager::beginDebugGroup(const std::string &str) {
    if (m_features.debugMarker) {
        VkDebugMarkerMarkerInfoEXT markerInfo = {};
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        markerInfo.pMarkerName = str.c_str();
        markerInfo.color[2] = 1.0f;
        markerInfo.color[3] = 1.0f;

        m_functions.CmdDebugMarkerBeginEXT(currentFrame().primaryCmdBuf->handle(), &markerInfo);
    }
}

/** End the current debug group. */
void VulkanGPUManager::endDebugGroup() {
    if (m_features.debugMarker)
        m_functions.CmdDebugMarkerEndEXT(currentFrame().primaryCmdBuf->handle());
}

#endif
