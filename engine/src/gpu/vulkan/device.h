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
 * @brief               Vulkan device class.
 */

#pragma once

#include "vulkan.h"

/** Class wrapping a logical device. */
class VulkanDevice : public VulkanHandle<VkDevice> {
public:
    VulkanDevice(VulkanGPUManager *manager, VkPhysicalDevice physicalDevice);
    ~VulkanDevice();

    bool identify(VulkanSurface *surface);
    bool isBetterThan(const VulkanDevice *other) const;
    void init();

    /** @return             Queue family to use. */
    uint32_t queueFamily() const { return m_queueFamily; }
    /** @return             Physical device handle. */
    VkPhysicalDevice physicalHandle() const { return m_physicalHandle; }

    /** @return             Device properties. */
    const VkPhysicalDeviceProperties &properties() const { return m_properties; }
    /** @return             Device limits. */
    const VkPhysicalDeviceLimits &limits() const { return m_properties.limits; }
private:
    uint32_t m_queueFamily;                 /**< Queue family to use. */
    VkPhysicalDevice m_physicalHandle;      /**< Physical device handle. */

    /** Physical device properties. */
    VkPhysicalDeviceProperties m_properties;

    /** Enabled device extensions. */
    std::vector<const char *> m_extensions;
};
