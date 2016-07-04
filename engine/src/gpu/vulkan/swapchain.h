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
class VulkanSwapchain {
public:
    /** Structure containing buffer details. */
    struct Buffer {
        VkImage image;                  /**< Image handle. */
        VkImageView view;               /**< View to the image. */
    };

    VulkanSwapchain(VulkanDevice *device, VulkanSurface *surface);
    ~VulkanSwapchain();

    void recreate();

    /** @return             Handle to the swap chain. */
    VkSwapchainKHR handle() const { return m_handle; }
    /** @return             Colour image format. */
    VkFormat colourFormat() const { return m_colourFormat; }
    /** @return             Current image details. */
    const Buffer &currentImage() const { return m_images[m_currentImage]; }

    void startFrame();
    void endFrame();
private:
    void clearImages();

    VulkanDevice *m_device;             /**< Device the swap chain is for. */
    VulkanSurface *m_surface;           /**< Surface the swap chain is for. */

    VkSwapchainKHR m_handle;            /**< Handle to the swap chain. */
    VkFormat m_colourFormat;            /**< Format of the colour images. */
    std::vector<Buffer> m_images;       /**< Array of image handles. */
    uint32_t m_currentImage;            /**< Current image index. */

    /**
     * Semaphore signalled when presentation is complete.
     *
     * This semaphore is passed to vkAcquireNextImageKHR(). It will become
     * signalled once presentation is completed, i.e. when the image is actually
     * usable.
     */
    VulkanSemaphore m_presentCompleteSem;
};
