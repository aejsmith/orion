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

#include "commands.h"
#include "manager.h"

#include "engine/engine.h"

/** Create a new command list for a render pass.
 * @param manager       Manager that owns the command list.
 * @param passInstance  Render pass instance pointer.
 * @param framebuffer   Framebuffer for the render pass. */
VulkanCommandList::VulkanCommandList(
    VulkanGPUManager *manager,
    GPURenderPassInstance *passInstance,
    const VulkanFramebuffer *framebuffer)
    :
    GPUCommandList(passInstance),
    VulkanObject(manager),
    m_cmdState(m_state)
{
    m_cmdState.renderPass = static_cast<const VulkanRenderPass *>(m_passInstance->desc().pass);
    m_cmdState.framebuffer = framebuffer;
}

/** Create a new child command list.
 * @param manager       Manager that owns the command list.
 * @param parent        Parent command list.
 * @param inherit       Flags indicating which state to inherit. */
VulkanCommandList::VulkanCommandList(VulkanGPUManager *manager, GPUCommandList *parent, uint32_t inherit) :
    GPUCommandList(parent, inherit),
    VulkanObject(manager),
    m_cmdState(m_state)
{
    auto vkParent = static_cast<VulkanCommandList *>(parent);
    m_cmdState.renderPass = vkParent->m_cmdState.renderPass;
    m_cmdState.framebuffer = vkParent->m_cmdState.framebuffer;
}

/** Destroy the command list. */
VulkanCommandList::~VulkanCommandList() {}

/** Create a child command list.
 * @param inherit       Flags indicating which state to inherit.
 * @return              Created command list. */
GPUCommandList *VulkanCommandList::createChild(uint32_t inherit) {
    return new VulkanCommandList(manager(), this, inherit);
}

/** Submit a child command list.
 * @param cmdList       Command list to submit. */
void VulkanCommandList::submitChild(GPUCommandList *cmdList) {
    /* End our current command list if any, we need to continue any other
     * commands after the child's command lists. */
    if (m_cmdState.cmdBuf) {
        m_cmdState.cmdBuf->end();
        m_cmdState.cmdBuf = nullptr;
    }

    /* Copy the command buffer list onto the end of ours. */
    auto vkCmdList = static_cast<VulkanCommandList *>(cmdList);
    m_cmdBufs.splice(m_cmdBufs.end(), vkCmdList->m_cmdBufs);

    delete cmdList;
}

/** Submit the command list to a primary command buffer.
 * @param primaryBuf    Command buffer to submit to. */
void VulkanCommandList::submit(VulkanCommandBuffer *primaryBuf) {
    if (m_cmdState.cmdBuf)
        m_cmdState.cmdBuf->end();

    std::vector<VkCommandBuffer> buffers;
    buffers.reserve(m_cmdBufs.size());
    for (VulkanCommandBuffer *secondaryBuf : m_cmdBufs) {
        buffers.push_back(secondaryBuf->handle());
        secondaryBuf->m_state = VulkanCommandBuffer::State::kSubmitted;
    }

    if (buffers.size())
        vkCmdExecuteCommands(primaryBuf->handle(), buffers.size(), &buffers[0]);
}

/** Create the command buffer ready for a command. */
void VulkanCommandList::prepareCmdBuf() {
    if (!m_cmdState.cmdBuf) {
        m_cmdState.cmdBuf = manager()->commandPool()->allocateTransient(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

        VkCommandBufferInheritanceInfo inheritanceInfo = {};
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.renderPass = m_cmdState.renderPass->handle();
        inheritanceInfo.framebuffer = m_cmdState.framebuffer->handle();
        m_cmdState.cmdBuf->begin(
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            &inheritanceInfo);

        m_cmdBufs.push_back(m_cmdState.cmdBuf);

        /* Reset our state as this will need to be set again on the new command
         * buffer. */
        m_cmdState.pipelineObject = VK_NULL_HANDLE;
        for (size_t i = 0; i < m_cmdState.descriptorSets.size(); i++)
            m_cmdState.descriptorSets[i] = VK_NULL_HANDLE;
    }
}

/** Draw primitives.
 * @param type          Primitive type to render.
 * @param vertices      Vertex data to use.
 * @param indices       Index data to use (can be null). */
void VulkanCommandList::draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) {
    check(m_cmdState.pending.pipeline);

    prepareCmdBuf();

    if (m_cmdState.pipeline != m_cmdState.pending.pipeline) {
        auto vkPipeline = static_cast<VulkanPipeline *>(m_cmdState.pending.pipeline.get());

        /* Binding a new pipeline may invalidate descriptor set bindings due to
         * layout incompatibilities. Clear out any which will become invalid so
         * that we rebind them. */
        size_t i = 0;
        if (m_cmdState.pipeline) {
            while (m_cmdState.pipeline->isCompatibleForSet(vkPipeline, i))
                i++;
        }
        for ( ; i < m_cmdState.descriptorSets.size(); i++)
            m_cmdState.descriptorSets[i] = VK_NULL_HANDLE;

        m_cmdState.pipeline = vkPipeline;
    }

    /* Get and bind a pipeline matching the current state. */
    m_cmdState.pipeline->bind(m_cmdState, type, vertices);

    /* Set viewport state. */
    if (m_dirtyState & kViewportState) {
        const IntRect &rect = m_cmdState.pending.viewport;

        /* Our viewport origin conventions match, but in Vulkan we render upside
         * down to compensate for clip space differences, so we must flip. */
        VkViewport viewport;
        viewport.x = rect.x;
        viewport.y = m_cmdState.framebuffer->size().y - (rect.y + rect.height);
        viewport.width = rect.width;
        viewport.height = rect.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vkCmdSetViewport(m_cmdState.cmdBuf->handle(), 0, 1, &viewport);
    }

    /* Set scissor state. */
    if (m_dirtyState & kScissorState) {
        const IntRect &rect = (m_cmdState.pending.scissorEnabled)
            ? m_cmdState.pending.scissor
            : m_cmdState.pending.viewport;

        /* Same as for viewport. */
        VkRect2D scissor;
        scissor.offset.x = rect.x;
        scissor.offset.y = m_cmdState.framebuffer->size().y - (rect.y + rect.height);
        scissor.extent.width = rect.width;
        scissor.extent.height = rect.height;

        vkCmdSetScissor(m_cmdState.cmdBuf->handle(), 0, 1, &scissor);
    }

    /* Bind resource sets. */
    const GPUResourceSetLayoutArray &resourceLayout = m_cmdState.pipeline->resourceLayout();
    for (size_t i = 0; i < m_cmdState.descriptorSets.size(); i++) {
        if (!resourceLayout[i] || !m_cmdState.pending.resourceSets[i])
            continue;

        auto vkResources = static_cast<VulkanResourceSet *>(m_cmdState.pending.resourceSets[i].get());
        vkResources->bind(m_cmdState, i);
    }

    /* Bind vertex buffers. */
    if (vertices->buffers().size()) {
        std::vector<VkBuffer> vertexBuffers(vertices->buffers().size());
        std::vector<VkDeviceSize> vertexBufferOffsets(vertices->buffers().size());
        for (size_t i = 0; i < vertices->buffers().size(); i++) {
            auto buffer = static_cast<VulkanBuffer *>(vertices->buffers()[i].get());
            vertexBuffers[i] = buffer->allocation()->buffer();
            vertexBufferOffsets[i] = buffer->allocation()->offset();

            m_cmdState.cmdBuf->addReference(buffer);
        }

        vkCmdBindVertexBuffers(
            m_cmdState.cmdBuf->handle(),
            0,
            vertexBuffers.size(),
            &vertexBuffers[0],
            &vertexBufferOffsets[0]);
    }

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

        auto buffer = static_cast<VulkanBuffer *>(indices->buffer());
        vkCmdBindIndexBuffer(
            m_cmdState.cmdBuf->handle(),
            buffer->allocation()->buffer(),
            buffer->allocation()->offset(),
            indexType);

        m_cmdState.cmdBuf->addReference(buffer);
    }

    m_dirtyState = 0;
    m_dirtyResourceSets = 0;

    /* Perform the draw! */
    if (indices) {
        vkCmdDrawIndexed(m_cmdState.cmdBuf->handle(), indices->count(), 1, indices->offset(), 0, 0);
    } else {
        vkCmdDraw(m_cmdState.cmdBuf->handle(), vertices->count(), 1, 0, 0);
    }

    g_engine->stats().drawCalls++;
}

#ifdef ORION_BUILD_DEBUG

/** Begin a debug group.
 * @param str           Group string. */
void VulkanGPUManager::beginDebugGroup(const std::string &str) {
    if (m_features.debugMarker) {
        VkDebugMarkerMarkerInfoEXT markerInfo = {};
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        markerInfo.pMarkerName = str.c_str();
        markerInfo.color[1] = 1.0f;
        markerInfo.color[3] = 1.0f;

        m_device->functions().CmdDebugMarkerBeginEXT(currentFrame().primaryCmdBuf->handle(), &markerInfo);
    }
}

/** End the current debug group. */
void VulkanGPUManager::endDebugGroup() {
    if (m_features.debugMarker)
        m_device->functions().CmdDebugMarkerEndEXT(currentFrame().primaryCmdBuf->handle());
}

/** Begin a debug group.
 * @param str           Group string. */
void VulkanCommandList::beginDebugGroup(const std::string &str) {
    if (manager()->features().debugMarker) {
        prepareCmdBuf();

        VkDebugMarkerMarkerInfoEXT markerInfo = {};
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        markerInfo.pMarkerName = str.c_str();
        markerInfo.color[1] = 0.5f;
        markerInfo.color[2] = 1.0f;
        markerInfo.color[3] = 1.0f;

        manager()->device()->functions().CmdDebugMarkerBeginEXT(m_cmdState.cmdBuf->handle(), &markerInfo);
    }
}

/** End the current debug group. */
void VulkanCommandList::endDebugGroup() {
    if (manager()->features().debugMarker) {
        prepareCmdBuf();
        manager()->device()->functions().CmdDebugMarkerEndEXT(m_cmdState.cmdBuf->handle());
    }
}

#endif