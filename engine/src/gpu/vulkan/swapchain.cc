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

#include "queue.h"
#include "surface.h"
#include "swapchain.h"

#include "engine/engine.h"

/** Number of swapchain images we want to create. */
static const uint32_t kNumSwapchainImages = 3;

/** Create a swap chain.
 * @param device        Device the swap chain is for.
 * @param surface       Surface the swap chain is for. */
VulkanSwapchain::VulkanSwapchain(VulkanDevice *device, VulkanSurface *surface) :
    m_device(device),
    m_surface(surface),
    m_handle(VK_NULL_HANDLE),
    m_currentImage(UINT32_MAX),
    m_presentCompleteSem(device)
{
    recreate();
}

/** Destroy the swap chain. */
VulkanSwapchain::~VulkanSwapchain() {
    clearImages();
    vkDestroySwapchainKHR(m_device->handle(), m_handle, nullptr);
}

/** Free existing images. */
void VulkanSwapchain::clearImages() {
    for (Buffer &buffer : m_images)
        vkDestroyImageView(m_device->handle(), buffer.view, nullptr);
}

/** (Re)create the swap chain. */
void VulkanSwapchain::recreate() {
    VkResult result;
    uint32_t count;

    VkSwapchainKHR oldSwapchain = m_handle;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface->handle();
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    /* Get surface capabilities. */
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        m_device->physicalHandle(),
        m_surface->handle(),
        &surfaceCapabilities);
    if (result != VK_SUCCESS)
        fatal("Failed to get Vulkan surface capabilities: %d", result);

    /* Determine number of images. Request at least one more than the minimum
     * number of images required by the presentation engine, because that is
     * the minimum it needs to work and we want an additional one for buffering. */
    createInfo.minImageCount = glm::clamp(
        kNumSwapchainImages,
        surfaceCapabilities.minImageCount + 1,
        surfaceCapabilities.maxImageCount);

    /* Define swap chain image extents. If the current extent is given as -1,
     * this means the surface size will be determined by the size we give for
     * the swap chain, so set the requested window size in this case. Otherwise,
     * use what we are given. */
    if (surfaceCapabilities.currentExtent.width == static_cast<uint32_t>(-1)) {
        createInfo.imageExtent.width = glm::clamp(
            m_surface->width(),
            surfaceCapabilities.minImageExtent.width,
            surfaceCapabilities.maxImageExtent.width);
        createInfo.imageExtent.height = glm::clamp(
            m_surface->height(),
            surfaceCapabilities.minImageExtent.height,
            surfaceCapabilities.maxImageExtent.height);
    } else {
        createInfo.imageExtent = surfaceCapabilities.currentExtent;
    }

    /* Determine presentation transformation. */
    createInfo.preTransform =
        (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
            : surfaceCapabilities.currentTransform;

    createInfo.imageColorSpace = m_surface->surfaceFormat().colorSpace;
    createInfo.imageFormat = m_surface->surfaceFormat().format;

    /* Get presentation modes. */
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_device->physicalHandle(),
        m_surface->handle(),
        &count,
        nullptr);
    if (result != VK_SUCCESS) {
        fatal("Failed to get Vulkan presentation modes (1): %d", result);
    } else if (count == 0) {
        fatal("No Vulkan presentation modes");
    }

    std::vector<VkPresentModeKHR> presentModes(count);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_device->physicalHandle(),
        m_surface->handle(),
        &count,
        &presentModes[0]);
    if (result != VK_SUCCESS)
        fatal("Failed to get Vulkan presentation modes (2): %d", result);

    /* FIFO mode (v-sync) should always be present. */
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    if (!g_engine->config().displayVsync) {
        /* If we don't want v-sync, try to find a mailbox mode, which is the
         * lowest latency non-tearing mode available. Failing that, pick an
         * immediate mode. */
        for (size_t i = 0; i < presentModes.size(); i++) {
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            if (createInfo.presentMode != VK_PRESENT_MODE_MAILBOX_KHR &&
                presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    result = vkCreateSwapchainKHR(m_device->handle(), &createInfo, nullptr, &m_handle);
    if (result != VK_SUCCESS)
        fatal("Failed to create Vulkan swap chain: %d", result);

    if (oldSwapchain != VK_NULL_HANDLE) {
        clearImages();
        vkDestroySwapchainKHR(m_device->handle(), oldSwapchain, nullptr);
    }

    /* Get an array of images. */
    result = vkGetSwapchainImagesKHR(m_device->handle(), m_handle, &count, nullptr);
    if (result != VK_SUCCESS)
        fatal("Failed to get Vulkan swap chain images (1): %d", result);
    check(count);

    std::vector<VkImage> images(count);
    result = vkGetSwapchainImagesKHR(m_device->handle(), m_handle, &count, &images[0]);
    if (result != VK_SUCCESS)
        fatal("Failed to get Vulkan swap chain images (2): %d", result);

    /* Prepare image view state. */
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = m_surface->surfaceFormat().format;
    viewCreateInfo.components = {
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A
    };
    viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    m_images.resize(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        m_images[i].image = images[i];

        /* Create an image view. */
        viewCreateInfo.image = images[i];
        checkVk(vkCreateImageView(m_device->handle(), &viewCreateInfo, nullptr, &m_images[i].view));
    }
}

/**
 * Start a new frame.
 *
 * Acquires the next image from the swap chain and transitions it back to the
 * colour attachment layout ready for use. The frame's target image can be
 * obtained by calling currentImage().
 */
void VulkanSwapchain::startFrame() {
    check(m_currentImage == UINT32_MAX);

    /* Get the next image from the presentation engine. This will wait
     * indefinitely until an image is available. The image however may not
     * actually be usable for rendering until the semaphore is signalled. */
    checkVk(vkAcquireNextImageKHR(
        m_device->handle(),
        m_handle,
        UINT64_MAX,
        m_presentCompleteSem.handle(),
        VK_NULL_HANDLE,
        &m_currentImage));
}

/**
 * End the current frame.
 *
 * Executes an image memory barrier on the current image, transitioning it to
 * the correct layout for presentation, and presents it to the window system.
 */
void VulkanSwapchain::endFrame() {
    /* Present the image. */
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_handle;
    presentInfo.pImageIndices = &m_currentImage;

    checkVk(vkQueuePresentKHR(m_device->queue()->handle(), &presentInfo));

    m_currentImage = UINT32_MAX;
}
