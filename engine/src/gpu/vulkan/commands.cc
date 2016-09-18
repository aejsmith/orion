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
#include "manager.h"

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
    check(currentFrame().renderPass);

    currentFrame().blendState = state;
}

/** Set the depth/stencil state.
 * @param state         Depth/stencil state to set. */
void VulkanGPUManager::setDepthStencilState(GPUDepthStencilState *state) {
    check(currentFrame().renderPass);

    currentFrame().depthStencilState = state;
}

/** Set the rasterizer state.
 * @param state         Rasterizer state to set. */
void VulkanGPUManager::setRasterizerState(GPURasterizerState *state) {
    check(currentFrame().renderPass);

    currentFrame().rasterizerState = state;
}

/** Set the viewport.
 * @param viewport      Viewport rectangle in pixels. */
void VulkanGPUManager::setViewport(const IntRect &viewport) {
    check(currentFrame().renderPass);

    currentFrame().viewport = viewport;
}

/** Set the scissor test parameters.
 * @param enable        Whether to enable scissor testing.
 * @param scissor       Scissor rectangle. */
void VulkanGPUManager::setScissor(bool enable, const IntRect &scissor) {
    check(currentFrame().renderPass);

    currentFrame().scissorEnabled = enable;
    currentFrame().scissor = scissor;
}

/** Draw primitives.
 * @param type          Primitive type to render.
 * @param vertices      Vertex data to use.
 * @param indices       Index data to use (can be null). */
void VulkanGPUManager::draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) {
    VulkanFrame &frame = currentFrame();

    check(frame.renderPass);
    check(frame.pipeline);

    VulkanCommandBuffer *cmdBuf = frame.primaryCmdBuf;

    if (frame.pipeline != frame.boundPipeline) {
        /* Binding a new pipeline may invalidate descriptor set bindings due to
         * layout incompatibilities. Clear out any which will become invalid so
         * that we rebind them. */
        size_t i = 0;
        if (frame.boundPipeline) {
            while (frame.boundPipeline->isCompatibleForSet(frame.pipeline, i))
                i++;
        }
        for ( ; i < frame.boundDescriptorSets.size(); i++)
            frame.boundDescriptorSets[i] = VK_NULL_HANDLE;

        frame.boundPipeline = frame.pipeline;
    }

    // TODO: Create and bind real pipeline object based on current state.
    // Reference vertex/index buffers.

    /* Bind resource sets. */
    const GPUResourceSetLayoutArray &resourceLayout = frame.boundPipeline->resourceLayout();
    for (size_t i = 0; i < frame.resourceSets.size(); i++) {
        if (!resourceLayout[i] || !frame.resourceSets[i])
            continue;

        VkDescriptorSet descriptorSet = frame.resourceSets[i]->prepareForDraw(cmdBuf);
        if (descriptorSet != frame.boundDescriptorSets[i]) {
            /* TODO: Maybe could bundle these up? It doesn't allow us to pass
             * in a sparse set of bindings though. */
            vkCmdBindDescriptorSets(
                cmdBuf->handle(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                frame.boundPipeline->layout(),
                i,
                1, &descriptorSet,
                0, nullptr);

            frame.boundDescriptorSets[i] = descriptorSet;
        }
    }

    #if 0
    m_memoryManager->flushStagingCmdBuf();
    endRenderPass();
    cmdBuf->end();
    m_queue->submit(cmdBuf);
    vkDeviceWaitIdle(m_device->handle());
    #endif

    fatal("TODO: draw");
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
