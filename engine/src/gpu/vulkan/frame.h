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
 * @brief               Vulkan per-frame data.
 */

#pragma once

#include "memory_manager.h"
#include "pipeline.h"
#include "resource.h"
#include "utility.h"

/** Structure tracking per-frame data for cleanup once the frame completes. */
struct VulkanFrame {
    /** Fence signalled upon completion of the frame's submission. */
    VulkanFence fence;

    /** Primary command buffer for the current frame. */
    VulkanCommandBuffer *primaryCmdBuf;

    /** List of command buffers allocated for the frame. */
    std::list<VulkanCommandBuffer *> cmdBuffers;

    /** List of staging memory allocations for the frame. */
    std::list<VulkanMemoryManager::StagingMemory *> stagingAllocations;

    /**
     * Rendering state.
     */

    /** Bound pipeline. */
    GPUObjectPtr<VulkanPipeline> pipeline;

    /** Bound resource sets. */
    std::array<GPUObjectPtr<VulkanResourceSet>, ResourceSets::kNumResourceSets> resourceSets;

    /** State object bindings. */
    GPUBlendStatePtr blendState;
    GPUDepthStencilStatePtr depthStencilState;
    GPURasterizerStatePtr rasterizerState;

    /** Viewport. */
    IntRect viewport;

    /** Scissor state. */
    bool scissorEnabled;
    IntRect scissor;

    /** Initialise the frame.
     * @param manager       Manager that owns the frame. */
    explicit VulkanFrame(VulkanGPUManager *manager) :
        fence(manager)
    {}
};
