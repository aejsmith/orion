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
 * @brief               Vulkan utility classes/functions.
 */

#pragma once

#include "vulkan.h"

/** Class wrapping a Vulkan fence. */
class VulkanFence : public VulkanHandle<VkFence> {
public:
    explicit VulkanFence(VulkanGPUManager *manager, bool signalled = false);
    ~VulkanFence();

    bool getStatus() const;
    bool wait(uint64_t timeout = UINT64_MAX) const;
};

/** Class wrapping a Vulkan semaphore. */
class VulkanSemaphore : public VulkanHandle<VkSemaphore> {
public:
    explicit VulkanSemaphore(VulkanGPUManager *manager);
    ~VulkanSemaphore();
};

namespace VulkanUtil {
    extern void setImageLayout(
        VulkanCommandBuffer *cmdBuf,
        VkImage image,
        const VkImageSubresourceRange &subresources,
        VkImageLayout oldLayout,
        VkImageLayout newLayout);
    extern void setImageLayout(
        VulkanCommandBuffer *cmdBuf,
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldLayout,
        VkImageLayout newLayout);

    /** Determine the aspect mask covering a given format.
     * @param format        Pixel format.
     * @return              Aspect mask. */
    inline VkImageAspectFlags aspectMaskForFormat(PixelFormat format) {
        VkImageAspectFlags aspectMask;

        if (PixelFormat::isDepth(format)) {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (PixelFormat::isDepthStencil(format))
                aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        } else {
            aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        return aspectMask;
    }
}
