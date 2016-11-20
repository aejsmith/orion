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

#pragma once

#include "command_buffer.h"
#include "pipeline.h"
#include "render_pass.h"

/**
 * Structuring containing command state.
 *
 * This structure contains the current state for a VulkanCommandList, kept
 * separately as we need to pass it around in various places. The state given
 * by GPUCommandList::State can be considered the "pending" state, i.e. it is
 * not actually applied at the Vulkan level until it is needed for a command.
 * This structure records the state that is actually applied on a command
 * buffer.
 */
struct VulkanCommandState {
    const GPUCommandList::State &pending;   /**< Pending state. */
    VulkanCommandBuffer *cmdBuf;            /**< Current command buffer. */
    const VulkanRenderPass *renderPass;     /**< Render pass. */
    const VulkanFramebuffer *framebuffer;   /**< Framebuffer for the pass. */

    /** Pipeline currently bound on the command buffer. */
    GPUObjectPtr<VulkanPipeline> pipeline;
    /** Underlying pipeline object bound on the command buffer. */
    VkPipeline pipelineObject;

    /** Descriptor sets actually bound on the command buffer. */
    std::array<VkDescriptorSet, ResourceSets::kNumResourceSets> descriptorSets;

    explicit VulkanCommandState(const GPUCommandList::State &inPending) :
        pending(inPending),
        cmdBuf(nullptr)
    {}
};

/** Vulkan command list implementation. */
class VulkanCommandList : public GPUCommandList, public VulkanObject {
public:
    VulkanCommandList(
        VulkanGPUManager *manager,
        GPURenderPassInstance *passInstance,
        const VulkanFramebuffer *framebuffer);
    VulkanCommandList(VulkanGPUManager *manager, GPUCommandList *parent, uint32_t inherit);
    ~VulkanCommandList();

    GPUCommandList *createChild(uint32_t inherit) override;
    void submitChild(GPUCommandList *cmdList) override;

    void draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) override;

    #ifdef ORION_BUILD_DEBUG
    void beginDebugGroup(const std::string &str) override;
    void endDebugGroup() override;
    #endif

    /**
     * Internal methods.
     */

    void submit(VulkanCommandBuffer *primaryBuf);

    /** @return             Command state. */
    const VulkanCommandState &cmdState() const { return m_cmdState; }
private:
    void prepareCmdBuf();

    VulkanCommandState m_cmdState;          /**< Internal state. */

    /**
     * List of command buffers.
     *
     * The high level command list interface allows for an arbitrary hierarchy
     * of command lists. Vulkan, however, only has primary and secondary
     * command buffers, and you cannot have a secondary command buffer execute
     * another one. What we do then is flatten out the command list hierarchy.
     * We have a single per-frame primary command buffer, which render pass
     * begin/end commands and any other commands outside render passes are
     * recorded on. Command lists are used for the contents of render passes and
     * are always secondary command buffers. When a child command list is
     * submitted to its parent, we end the parent's current command list (if
     * any) and splice the child's buffer list on to the end of the parent's.
     * If the parent tries to record any more commands, we begin a new command
     * buffer for it. At the end of a render pass, that leaves us with a flat
     * list of secondary command buffers in the right order, which we submit
     * with vkCmdExecuteCommands().
     *
     * The current command buffer that this list is recording to is found in
     * m_cmdState->cmdBuf.
     */
    std::list<VulkanCommandBuffer *> m_cmdBufs;
};
