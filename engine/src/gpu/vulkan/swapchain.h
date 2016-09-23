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
 * @brief               Vulkan swapchain class.
 */

#pragma once

#include "utility.h"

class VulkanSurface;

/** Class wrapping a Vulkan swap chain. */
class VulkanSwapchain : public VulkanHandle<VkSwapchainKHR> {
public:
    explicit VulkanSwapchain(VulkanGPUManager *manager);
    ~VulkanSwapchain();

    void recreate();

    /** @return             Current image details. */
    VkImage currentImage() const { return m_images[m_currentImage]; }
    /** @return             Presentation completion semaphore. */
    VulkanSemaphore &presentCompleteSem() { return m_presentCompleteSem; }
    /** @return             Rendering completion semaphore. */
    VulkanSemaphore &renderCompleteSem() { return m_renderCompleteSem; }

    void startFrame();
    void endFrame();
private:
    std::vector<VkImage> m_images;      /**< Array of image handles. */
    uint32_t m_currentImage;            /**< Current image index. */

    /**
     * Semaphore signalled when presentation is complete.
     *
     * This semaphore is passed to vkAcquireNextImageKHR(). It will become
     * signalled once presentation is completed, i.e. when the image is actually
     * usable. This semaphore must be waited on before a new frame's command
     * buffer starts executing.
     */
    VulkanSemaphore m_presentCompleteSem;

    /**
     * Semaphore signalled when rendering is complete.
     *
     * This semaphore is passed to vkQueuePresentKHR(). It must be signalled
     * after the frame's command buffer is completed to indicate that the
     * frame can be presented.
     */
    VulkanSemaphore m_renderCompleteSem;
};
