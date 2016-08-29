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

#include "device.h"

/** Class wrapping a Vulkan fence. */
class VulkanFence {
public:
    /** Create a new fence.
     * @param device        Device to create the semaphore on.
     * @param signalled     Whether the fence should begin in the signalled
     *                      state (defaults to false). */
    explicit VulkanFence(VulkanDevice *device, bool signalled = false) :
        m_device(device)
    {
        VkFenceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        if (signalled)
            createInfo.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
        checkVk(vkCreateFence(m_device->handle(), &createInfo, nullptr, &m_handle));
    }

    /** Destroy the fence. */
    ~VulkanFence() {
        vkDestroyFence(m_device->handle(), m_handle, nullptr);
    }

    /** @return             Handle to the fence. */
    VkFence handle() const { return m_handle; }

    /** Get the fence status.
     * @return              Whether the fence is signalled. */
    bool getStatus() const {
        VkResult result = vkGetFenceStatus(m_device->handle(), m_handle);
        switch (result) {
            case VK_SUCCESS:
                return true;
            case VK_NOT_READY:
                return false;
            default:
                unreachable();
        }
    }
private:
    VulkanDevice *m_device;             /**< Device the fence belongs to. */
    VkFence m_handle;                   /**< Handle to the fence. */
};

/** Class wrapping a Vulkan semaphore. */
class VulkanSemaphore {
public:
    /** Create a new semaphore.
     * @param device        Device to create the semaphore on. */
    explicit VulkanSemaphore(VulkanDevice *device) :
        m_device(device)
    {
        VkSemaphoreCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        checkVk(vkCreateSemaphore(m_device->handle(), &createInfo, nullptr, &m_handle));
    }

    /** Destroy the semaphore. */
    ~VulkanSemaphore() {
        vkDestroySemaphore(m_device->handle(), m_handle, nullptr);
    }

    /** @return             Handle to the semaphore. */
    VkSemaphore handle() const { return m_handle; }
private:
    VulkanDevice *m_device;             /**< Device the semaphore belongs to. */
    VkSemaphore m_handle;               /**< Handle to the semaphore. */
};
