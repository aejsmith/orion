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
 * @brief               Vulkan utility functions.
 */

#include "manager.h"
#include "utility.h"

/** Create a new fence.
 * @param manager       Manager that owns the fence.
 * @param signalled     Whether the fence should begin in the signalled state
 *                      (defaults to false). */
VulkanFence::VulkanFence(VulkanGPUManager *manager, bool signalled) :
    VulkanHandle(manager)
{
    VkFenceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (signalled)
        createInfo.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
    checkVk(vkCreateFence(manager->device()->handle(), &createInfo, nullptr, &m_handle));
}

/** Destroy the fence. */
VulkanFence::~VulkanFence() {
    vkDestroyFence(manager()->device()->handle(), m_handle, nullptr);
}

/** Get the fence status.
 * @return              Whether the fence is signalled. */
bool VulkanFence::getStatus() const {
    VULKAN_PROFILE_FUNCTION_SCOPE();

    VkResult result = vkGetFenceStatus(manager()->device()->handle(), m_handle);
    switch (result) {
        case VK_SUCCESS:
            return true;
        case VK_NOT_READY:
            return false;
        default:
            checkVk(result);
            return false;
    }
}

/** Wait for the fence.
 * @param timeout       Wait timeout (defaults to indefinite).
 * @return              Whether the fence was signalled within the timeout. */
bool VulkanFence::wait(uint64_t timeout) const {
    VULKAN_PROFILE_FUNCTION_SCOPE();

    VkResult result = vkWaitForFences(manager()->device()->handle(), 1, &m_handle, true, timeout);
    switch (result) {
        case VK_SUCCESS:
            return true;
        case VK_TIMEOUT:
            return false;
        default:
            checkVk(result);
            return false;
    }
}

/** Create a new semaphore.
 * @param manager       Manager that owns the semaphore. */
VulkanSemaphore::VulkanSemaphore(VulkanGPUManager *manager) :
    VulkanHandle (manager)
{
    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    checkVk(vkCreateSemaphore(manager->device()->handle(), &createInfo, nullptr, &m_handle));
}

/** Destroy the semaphore. */
VulkanSemaphore::~VulkanSemaphore() {
    vkDestroySemaphore(manager()->device()->handle(), m_handle, nullptr);
}

/** Set the layout of an image.
 * @param cmdBuf        Command buffer to use.
 * @param image         Image to set layout of.
 * @param subresources  Subresource range to set layout of.
 * @param oldLayout     Previous layout.
 * @param newLayout     New layout. */
void VulkanUtil::setImageLayout(VulkanCommandBuffer *cmdBuf,
                                VkImage image,
                                const VkImageSubresourceRange &subresources,
                                VkImageLayout oldLayout,
                                VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.image = image;
    barrier.subresourceRange = subresources;

    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    switch (oldLayout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        default:
            break;
    }

    switch (newLayout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            srcStageMask = (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                ? VK_PIPELINE_STAGE_TRANSFER_BIT
                : VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
        default:
            break;
    }

    vkCmdPipelineBarrier(cmdBuf->handle(),
                         srcStageMask,
                         dstStageMask,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
}

/** Set the layout of the first mip of the first layer of an image.
 * @param cmdBuf        Command buffer to use.
 * @param image         Image to set layout of.
 * @param subresources  Subresource range to set layout of.
 * @param oldLayout     Previous layout.
 * @param newLayout     New layout. */
void VulkanUtil::setImageLayout(VulkanCommandBuffer *cmdBuf,
                                VkImage image,
                                VkImageAspectFlags aspectMask,
                                VkImageLayout oldLayout,
                                VkImageLayout newLayout)
{
    VkImageSubresourceRange subresources = {};
    subresources.aspectMask = aspectMask;
    subresources.levelCount = 1;
    subresources.layerCount = 1;
    setImageLayout(cmdBuf, image, subresources, oldLayout, newLayout);
}
