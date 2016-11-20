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

#include "manager.h"
#include "swapchain.h"

#include "engine/engine.h"

/** Number of swapchain images we want to create. */
static const uint32_t kNumSwapchainImages = 3;

/** Create a swap chain.
 * @param manager       Manager that owns the swap chain. */
VulkanSwapchain::VulkanSwapchain(VulkanGPUManager *manager) :
    VulkanHandle(manager),
    m_currentImage(UINT32_MAX),
    m_currentSem(0)
{
    recreate();
}

/** Destroy the swap chain. */
VulkanSwapchain::~VulkanSwapchain() {
    vkDestroySwapchainKHR(manager()->device()->handle(), m_handle, nullptr);
}

/** (Re)create the swap chain. */
void VulkanSwapchain::recreate() {
    VkResult result;
    uint32_t count;

    VulkanDevice *device = manager()->device();
    VulkanSurface *surface = manager()->surface();

    VkSwapchainKHR oldSwapchain = m_handle;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->handle();
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    /* Get surface capabilities. */
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device->physicalHandle(),
        surface->handle(),
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
            surface->width(),
            surfaceCapabilities.minImageExtent.width,
            surfaceCapabilities.maxImageExtent.width);
        createInfo.imageExtent.height = glm::clamp(
            surface->height(),
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

    createInfo.imageColorSpace = surface->surfaceFormat().colorSpace;
    createInfo.imageFormat = surface->surfaceFormat().format;

    /* Get presentation modes. */
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device->physicalHandle(),
        surface->handle(),
        &count,
        nullptr);
    if (result != VK_SUCCESS) {
        fatal("Failed to get Vulkan presentation modes (1): %d", result);
    } else if (count == 0) {
        fatal("No Vulkan presentation modes");
    }

    std::vector<VkPresentModeKHR> presentModes(count);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device->physicalHandle(),
        surface->handle(),
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

    result = vkCreateSwapchainKHR(device->handle(), &createInfo, nullptr, &m_handle);
    if (result != VK_SUCCESS)
        fatal("Failed to create Vulkan swap chain: %d", result);

    if (oldSwapchain != VK_NULL_HANDLE) {
        m_images.clear();
        vkDestroySwapchainKHR(device->handle(), oldSwapchain, nullptr);
    }

    /* Get an array of images. */
    result = vkGetSwapchainImagesKHR(device->handle(), m_handle, &count, nullptr);
    if (result != VK_SUCCESS)
        fatal("Failed to get Vulkan swap chain images (1): %d", result);

    m_images.resize(count);
    m_presentCompleteSems.clear();
    m_renderCompleteSems.clear();

    for (uint32_t i = 0; i < count; i++) {
        m_presentCompleteSems.emplace_back(new VulkanSemaphore(manager()));
        m_renderCompleteSems.emplace_back(new VulkanSemaphore(manager()));
    }

    result = vkGetSwapchainImagesKHR(device->handle(), m_handle, &count, &m_images[0]);
    if (result != VK_SUCCESS)
        fatal("Failed to get Vulkan swap chain images (2): %d", result);
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
    m_currentSem = (m_currentSem + 1) % m_images.size();
    checkVk(vkAcquireNextImageKHR(
        manager()->device()->handle(),
        m_handle,
        UINT64_MAX,
        m_presentCompleteSems[m_currentSem]->handle(),
        VK_NULL_HANDLE,
        &m_currentImage));
}

/**
 * End the current frame.
 *
 * Transfers from the backbuffer to the current swapchain image, submits the
 * command buffer, and then presents the frame.
 *
 * @param cmdBuf        Command buffer to submit.
 * @param fence         Fence to signal when the command buffer has finished.
 */
void VulkanSwapchain::endFrame(VulkanCommandBuffer *cmdBuf, VulkanFence *fence) {
    auto texture = static_cast<VulkanTexture *>(manager()->surface()->texture());

    /* Blit to the swapchain image, flipping it in the process. */
    int32_t width = texture->width();
    int32_t height = texture->height();
    VkImageBlit imageBlit = {};
    imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.srcSubresource.layerCount = 1;
    imageBlit.srcOffsets[0] = { 0, 0, 0 };
    imageBlit.srcOffsets[1] = { width, height, 1 };
    imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.dstSubresource.layerCount = 1;
    imageBlit.dstOffsets[0] = { 0, height, 0 };
    imageBlit.dstOffsets[1] = { width, 0, 1 };

    /* Transition the surface image to the transfer source layout and the
     * swapchain image to transfer destination. */
    VulkanUtil::setImageLayout(
        cmdBuf,
        texture->handle(),
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VulkanUtil::setImageLayout(
        cmdBuf,
        m_images[m_currentImage],
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    /* Perform the blit. */
    vkCmdBlitImage(
        cmdBuf->handle(),
        texture->handle(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        m_images[m_currentImage],
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &imageBlit,
        VK_FILTER_NEAREST);

    /* Transition the surface image back to shader read only and the swapchain
     * image to present source. */
    VulkanUtil::setImageLayout(
        cmdBuf,
        texture->handle(),
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VulkanUtil::setImageLayout(
        cmdBuf,
        m_images[m_currentImage],
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    /* Submit the command buffer. Need to wait until presentation is completed
     * before executing, and need to signal the semaphore that the present will
     * wait on after execution. */
    cmdBuf->end();
    manager()->queue()->submit(
        cmdBuf,
        m_presentCompleteSems[m_currentSem].get(),
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        m_renderCompleteSems[m_currentSem].get(),
        fence);

    /* Present the image. */
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    VkSemaphore semaphore = m_renderCompleteSems[m_currentSem]->handle();
    presentInfo.pWaitSemaphores = &semaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_handle;
    presentInfo.pImageIndices = &m_currentImage;

    checkVk(vkQueuePresentKHR(manager()->queue()->handle(), &presentInfo));

    m_currentImage = UINT32_MAX;
}
