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
 * @brief               Vulkan per-frame state management.
 */

#include "command_buffer.h"
#include "frame.h"
#include "swapchain.h"

/** Begin a new frame. */
void VulkanGPUManager::startFrame() {
    /* Start the new frame. */
    m_frames.push_back(new VulkanFrame(this));
    VulkanFrame &frame = currentFrame();

    /* Allocate the primary command buffer. */
    frame.primaryCmdBuf = m_commandPool->allocateTransient();
    frame.primaryCmdBuf->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    /* Acquire a new image from the swap chain. */
    m_swapchain->startFrame();

    // TODO: Need to wait for present complete semaphore, transition layout.
}

/** End a frame and present it on screen. */
void VulkanGPUManager::endFrame() {
    // TODO: Transition image layout to present.
    VulkanFrame &completedFrame = currentFrame();

    m_memoryManager->flushStagingCmdBuf();
    completedFrame.primaryCmdBuf->end();

    m_swapchain->endFrame();

    /* Release all state. Probably a bit unnecessary because these have probably
     * been used for rendering and therefore have been referenced in a command
     * buffer anyway, but doesn't hurt to drop our references now. */
    completedFrame.pipeline = nullptr;
    completedFrame.blendState = nullptr;
    completedFrame.depthStencilState = nullptr;
    completedFrame.rasterizerState = nullptr;
    for (size_t i = 0; i < completedFrame.resourceSets.size(); i++)
        completedFrame.resourceSets[i] = nullptr;

    /* Clean up completed frames. */
    for (auto i = m_frames.begin(); i != m_frames.end(); ) {
        auto frame = *i;

        /* Check whether the frame has completed. */
        bool completed = frame->fence.getStatus();

        /* Perform cleanup work on the frame. */
        m_commandPool->cleanupFrame(*frame, completed);
        m_memoryManager->cleanupFrame(*frame, completed);

        /* Remove the frame if it has completed. */
        if (completed) {
            m_frames.erase(i++);
            delete frame;
        } else {
            ++i;
        }
    }

    /* Prepare state for the next frame. */
    startFrame();
}
