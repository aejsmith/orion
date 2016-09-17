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

#include "device.h"
#include "manager.h"

/** List of required device extensions. */
static const char *kRequiredDeviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

/** Initialise the device object (does not create device).
 * @param manager        Manager that owns this device.
 * @param physicalDevice Physical device this object corresponds to. */
VulkanDevice::VulkanDevice(VulkanGPUManager *manager, VkPhysicalDevice physicalDevice) :
    VulkanHandle(manager),
    m_physicalHandle(physicalDevice)
{}

/** Destroy the device. */
VulkanDevice::~VulkanDevice() {
    if (m_handle != VK_NULL_HANDLE)
        vkDestroyDevice(m_handle, nullptr);
}

/** Identify the device.
 * @param surface       Surface the device needs to support.
 * @return              Whether the device is suitable. */
bool VulkanDevice::identify(VulkanSurface *surface) {
    VkResult result;
    uint32_t count;

    vkGetPhysicalDeviceProperties(m_physicalHandle, &m_properties);

    const char *vendorString = "Unknown";
    switch (m_properties.vendorID) {
        case 0x8086:
            vendorString = "Intel";
            break;
        case 0x1002:
            vendorString = "AMD";
            break;
        case 0x10de:
            vendorString = "NVIDIA";
            break;
    }

    logInfo("    API version: %u.%u.%u",
        VK_VERSION_MAJOR(m_properties.apiVersion),
        VK_VERSION_MINOR(m_properties.apiVersion),
        VK_VERSION_PATCH(m_properties.apiVersion));
    logInfo("    Vendor:      0x%x (%s)", m_properties.vendorID, vendorString);
    logInfo("    Device:      0x%x (%s)", m_properties.deviceID, m_properties.deviceName);

    /* Query supported device extensions. */
    result = vkEnumerateDeviceExtensionProperties(m_physicalHandle, nullptr, &count, nullptr);
    if (result != VK_SUCCESS)
        fatal("Failed to enumerate Vulkan device extensions (1): %d", result);

    std::vector<VkExtensionProperties> extensionProps(count);
    result = vkEnumerateDeviceExtensionProperties(m_physicalHandle, nullptr, &count, &extensionProps[0]);
    if (result != VK_SUCCESS)
        fatal("Failed to enumerate Vulkan device extensions (2): %d", result);

    HashSet<std::string> availableExtensions;
    logInfo("    Extensions:");
    for (const auto &extension : extensionProps) {
        logInfo("      %s (revision %u)", extension.extensionName, extension.specVersion);
        availableExtensions.insert(extension.extensionName);
    }

    /* Check whether we have all required extensions, */
    m_extensions.assign(
        kRequiredDeviceExtensions,
        &kRequiredDeviceExtensions[arraySize(kRequiredDeviceExtensions)]);
    for (const char *extension : m_extensions) {
        if (availableExtensions.find(extension) == availableExtensions.end()) {
            logWarning("    Required device extension '%s' not available", extension);
            return false;
        }
    }

    /* Find suitable queue families. We need to support both graphics operations
     * and presentation to our surface. */
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalHandle, &count, nullptr);
    m_queueFamily = UINT32_MAX;
    if (count > 0) {
        std::vector<VkQueueFamilyProperties> queueFamilyProps(count);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalHandle, &count, &queueFamilyProps[0]);

        for (uint32_t i = 0; i < count; i++) {
            /* Check for graphics support. */
            bool graphicsSupported =
                queueFamilyProps[i].queueCount > 0 &&
                queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;

            /* Check support for presentation to our surface. */
            VkBool32 presentSupported = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                m_physicalHandle,
                i,
                surface->handle(),
                &presentSupported);

            if (graphicsSupported && presentSupported) {
                m_queueFamily = i;
                break;
            }
        }
    }

    if (m_queueFamily == UINT32_MAX) {
        logWarning("    No suitable queue families");
        return false;
    }

    return true;
}

/** Check whether this device is better than another.
 * @param other         Other device to check.
 * @return              Whether this device is better. */
bool VulkanDevice::isBetterThan(const VulkanDevice *other) const {
    // TODO: Implement this properly. Criteria that could be used here include
    // checking the type of the GPU (integrated vs discrete).
    return false;
}

/** Initialise the logical device. */
void VulkanDevice::init() {
    const float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    std::vector<const char *> layers;

    /* Assume that if the instance layers are available, the device layers are. */
    #if ORION_VULKAN_VALIDATION
        if (manager()->features().validation)
            layers.push_back("VK_LAYER_LUNARG_standard_validation");
    #endif

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledLayerCount = layers.size();
    deviceCreateInfo.ppEnabledLayerNames = &layers[0];
    deviceCreateInfo.enabledExtensionCount = m_extensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = &m_extensions[0];

    VkResult result = vkCreateDevice(m_physicalHandle, &deviceCreateInfo, nullptr, &m_handle);
    if (result != VK_SUCCESS)
        fatal("Failed to create Vulkan device: %d", result);
}
