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
    VulkanFrame &frame = currentFrame();

    check(frame.renderPass);

    if (frame.viewport != viewport) {
        frame.viewport = viewport;
        frame.viewportDirty = true;
    }
}

/** Set the scissor test parameters.
 * @param enable        Whether to enable scissor testing.
 * @param scissor       Scissor rectangle. */
void VulkanGPUManager::setScissor(bool enable, const IntRect &scissor) {
    VulkanFrame &frame = currentFrame();

    check(frame.renderPass);

    if (frame.scissorEnabled != enable || frame.scissor != scissor) {
        frame.scissorEnabled = enable;
        frame.scissor = scissor;
        frame.scissorDirty = true;
    }
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

    /* Get and bind a pipeline matching the current state. */
    VkPipeline pipeline = frame.boundPipeline->lookup(frame, type, vertices);
    if (pipeline != frame.boundPipelineObject) {
        vkCmdBindPipeline(cmdBuf->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        frame.boundPipelineObject = pipeline;

        /* Reference the object (will already have been done if already bound). */
        cmdBuf->addReference(frame.boundPipeline);
    }

    /* Set viewport state. */
    if (frame.viewportDirty) {
        VkViewport viewport;
        viewport.x = frame.viewport.x;
        viewport.y = frame.viewport.y;
        viewport.width = frame.viewport.width;
        viewport.height = frame.viewport.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vkCmdSetViewport(cmdBuf->handle(), 0, 1, &viewport);

        frame.viewportDirty = false;
    }

    /* Set scissor state. */
    if (frame.scissorDirty) {
        VkRect2D scissor;
        if (frame.scissorEnabled) {
            scissor.offset.x = frame.scissor.x;
            scissor.offset.y = frame.scissor.y;
            scissor.extent.width = frame.scissor.width;
            scissor.extent.height = frame.scissor.height;
        } else {
            scissor.offset.x = frame.viewport.x;
            scissor.offset.y = frame.viewport.y;
            scissor.extent.width = frame.viewport.width;
            scissor.extent.height = frame.viewport.height;
        }

        vkCmdSetScissor(cmdBuf->handle(), 0, 1, &scissor);

        frame.scissorDirty = false;
    }

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

    /* Bind vertex buffers. */
    std::vector<VkBuffer> vertexBuffers(vertices->buffers().size());
    std::vector<VkDeviceSize> vertexBufferOffsets(vertices->buffers().size());
    for (size_t i = 0; i < vertices->buffers().size(); i++) {
        VulkanBuffer *buffer = static_cast<VulkanBuffer *>(vertices->buffers()[i].get());
        vertexBuffers[i] = buffer->allocation()->buffer();
        vertexBufferOffsets[i] = buffer->allocation()->offset();

        // FIXME: Put both these calls into a helper on VulkanBuffer, it's an
        // implementation detail.
        cmdBuf->addReference(buffer);
        cmdBuf->addReference(buffer->allocation());
    }

    vkCmdBindVertexBuffers(
        cmdBuf->handle(),
        0,
        vertexBuffers.size(),
        &vertexBuffers[0],
        &vertexBufferOffsets[0]);

    /* Bind the index buffer. */
    if (indices) {
        VkIndexType indexType;
        switch (indices->type()) {
            case GPUIndexData::kUnsignedShortType:
                indexType = VK_INDEX_TYPE_UINT16;
                break;
            case GPUIndexData::kUnsignedIntType:
                indexType = VK_INDEX_TYPE_UINT32;
                break;
        }

        VulkanBuffer *buffer = static_cast<VulkanBuffer *>(indices->buffer());
        vkCmdBindIndexBuffer(
            cmdBuf->handle(),
            buffer->allocation()->buffer(),
            buffer->allocation()->offset(),
            indexType);

        cmdBuf->addReference(buffer);
        cmdBuf->addReference(buffer->allocation());
    }

    /* Perform the draw! */
    if (indices) {
        vkCmdDrawIndexed(cmdBuf->handle(), indices->count(), 1, indices->offset(), 0, 0);
    } else {
        vkCmdDraw(cmdBuf->handle(), vertices->count(), 1, 0, 0);
    }
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
