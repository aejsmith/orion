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
 * @brief               Vulkan GPU manager.
 */

#include "command_buffer.h"
#include "device.h"
#include "surface.h"
#include "swapchain.h"
#include "vulkan.h"

#include "core/hash_table.h"
#include "core/string.h"

#include "engine/engine.h"
#include "engine/window.h"

/** Global Vulkan GPU manager instance. */
VulkanGPUManager *g_vulkan;

/** List of required instance extensions. */
static const char *kRequiredInstanceExtensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
};

/** Create the GPU manager.
 * @param config        Engine configuration.
 * @param window        Where to store pointer to created window.
 * @return              Pointer to created GPU manager. */
GPUManager *GPUManager::create(const EngineConfiguration &config, Window *&window) {
    return new VulkanGPUManager(config, window);
}

/** Determine the instance layers/extensions to use.
 * @param window        Window (needed to determine required surface extension).
 * @param layers        Where to store array of layers to enable.
 * @param extensions    Where to store array of extensions to enable.
 * @param features      Features structure to fill in. */
static void enableInstanceExtensions(
    Window *window,
    std::vector<const char *> &layers,
    std::vector<const char *> &extensions,
    VulkanFeatures &features)
{
    VkResult result;
    uint32_t count;

    /* Enumerate available layers. */
    result = vkEnumerateInstanceLayerProperties(&count, nullptr);
    if (result != VK_SUCCESS)
        fatal("Failed to enumerate Vulkan instance layers (1): %d", result);

    std::vector<VkLayerProperties> layerProps(count);
    result = vkEnumerateInstanceLayerProperties(&count, &layerProps[0]);
    if (result != VK_SUCCESS)
        fatal("Failed to enumerate Vulkan instance layers (2): %d", result);

    HashSet<std::string> availableLayers;
    logInfo("  Instance layers:");
    for (const auto &layer : layerProps) {
        logInfo(
            "    %s (spec version %u.%u.%u, revision %u)",
            layer.layerName,
            VK_VERSION_MAJOR(layer.specVersion),
            VK_VERSION_MINOR(layer.specVersion),
            VK_VERSION_PATCH(layer.specVersion),
            layer.implementationVersion);
        availableLayers.insert(layer.layerName);
    }

    /* Enumerate available extensions. */
    result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if (result != VK_SUCCESS)
        fatal("Failed to enumerate Vulkan instance extensions (1): %d", result);

    std::vector<VkExtensionProperties> extensionProps(count);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &count, &extensionProps[0]);
    if (result != VK_SUCCESS)
        fatal("Failed to enumerate Vulkan instance extensions (2): %d", result);

    HashSet<std::string> availableExtensions;
    logInfo("  Instance extensions:");
    for (const auto &extension : extensionProps) {
        logInfo("    %s (revision %u)", extension.extensionName, extension.specVersion);
        availableExtensions.insert(extension.extensionName);
    }

    /* Check whether we have all required extensions, including the platform-
     * specific surface extension. */
    extensions.assign(
        kRequiredInstanceExtensions,
        &kRequiredInstanceExtensions[arraySize(kRequiredInstanceExtensions)]);
    extensions.push_back(VulkanSurface::getPlatformExtensionName(window));
    for (const char *extension : extensions) {
        if (availableExtensions.find(extension) == availableExtensions.end())
            fatal("Required Vulkan instance extension '%s' not available", extension);
    }

    /* Enable validation/debug extensions if requested and present. */
    #if ORION_VULKAN_VALIDATION
        auto validationLayer = availableLayers.find("VK_LAYER_LUNARG_standard_validation");
        auto reportExtension = availableExtensions.find(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

        if (validationLayer != availableLayers.end() && reportExtension != availableExtensions.end()) {
            layers.push_back("VK_LAYER_LUNARG_standard_validation");
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            features.validation = true;
        }
    #endif
}

#if ORION_VULKAN_VALIDATION

/** Vulkan debug report callback. */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char *pLayerPrefix,
    const char *pMessage,
    void *pUserData)
{
    LogLevel level = LogLevel::kDebug;
    std::string flagsString;

    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        flagsString += String::format("%sDEBUG", (!flagsString.empty()) ? " | " : "");
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        flagsString += String::format("%sINFORMATION", (!flagsString.empty()) ? " | " : "");
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        flagsString += String::format("%sWARNING", (!flagsString.empty()) ? " | " : "");
        level = LogLevel::kWarning;
    }
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        flagsString += String::format("%sPERFORMANCE", (!flagsString.empty()) ? " | " : "");
        level = LogLevel::kWarning;
    }
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        flagsString += String::format("%sERROR", (!flagsString.empty()) ? " | " : "");
        level = LogLevel::kError;
    }

    logWrite(level,
        "Vulkan [layer = %s, flags = %s, object = 0x%" PRIx64 ", location = %zu, messageCode = %d]:",
        pLayerPrefix, flagsString.c_str(), object, location, messageCode);
    logWrite(level, "  %s", pMessage);

    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        fatal("Vulkan validation error (see log for details)");

    return VK_FALSE;
}

#endif

/** Initialise the Vulkan GPU manager.
 * @param config        Engine configuration.
 * @param window        Where to store pointer to created window. */
VulkanGPUManager::VulkanGPUManager(const EngineConfiguration &config, Window *&window) :
    m_features(),
    m_primaryCmdBuf(nullptr)
{
    VkResult result;

    g_vulkan = this;

    /* Create the main window. */
    window = new Window(config, 0);

    logInfo("Initialising Vulkan");

    /* Determine the extensions to use. */
    std::vector<const char *> enabledLayers;
    std::vector<const char *> enabledExtensions;
    enableInstanceExtensions(window, enabledLayers, enabledExtensions, m_features);

    /* Create the instance. */
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = config.title.c_str();
    applicationInfo.pEngineName = "Orion";
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = enabledLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = &enabledLayers[0];
    instanceCreateInfo.enabledExtensionCount = enabledExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = &enabledExtensions[0];

    result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS)
        fatal("Failed to create Vulkan instance: %d", result);

    /* Get extension function pointers. */
    m_functions.init(m_instance, m_features);

    /* Register a debug report callback. */
    #if ORION_VULKAN_VALIDATION
        VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
        callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        callbackCreateInfo.flags =
            VK_DEBUG_REPORT_ERROR_BIT_EXT |
            VK_DEBUG_REPORT_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        callbackCreateInfo.pfnCallback = debugReportCallback;

        VkDebugReportCallbackEXT callback;
        m_functions.CreateDebugReportCallbackEXT(m_instance, &callbackCreateInfo, nullptr, &callback);
    #endif

    /* Create a surface for the main window. */
    m_surface = new VulkanSurface(window);

    /* Get a list of physical devices. */
    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (result != VK_SUCCESS) {
        fatal("Failed to enumerate Vulkan physical devices (1): %d", result);
    } else if (deviceCount == 0) {
        fatal("No Vulkan physical devices available");
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    result = vkEnumeratePhysicalDevices(m_instance, &deviceCount, &physicalDevices[0]);
    if (result != VK_SUCCESS)
        fatal("Failed to enumerate Vulkan physical devices (2): %d", result);

    /* From the devices which suit our needs, identify the best. */
    std::vector<std::unique_ptr<VulkanDevice>> devices(deviceCount);
    uint32_t bestDevice = UINT32_MAX;
    for (uint32_t i = 0; i < physicalDevices.size(); i++) {
        logInfo("  Device %u:", i);
        std::unique_ptr<VulkanDevice> device = std::make_unique<VulkanDevice>(physicalDevices[i]);
        if (device->identify(m_surface)) {
            if (bestDevice == UINT32_MAX || device->isBetterThan(devices[i].get()))
                bestDevice = i;

            devices[i] = std::move(device);
        }
    }

    if (bestDevice == UINT32_MAX)
        fatal("No suitable Vulkan physical device found");

    logInfo("  Using device %u", bestDevice);
    m_device = devices[bestDevice].release();
    devices.clear();

    /* Create the logical device. */
    m_device->init();

    /* Create a swapchain. */
    m_swapchain = new VulkanSwapchain(m_device, m_surface);
}

/** Shut down the Vulkan GPU manager. */
VulkanGPUManager::~VulkanGPUManager() {
    vkDeviceWaitIdle(m_device->handle());

    delete m_swapchain;
    delete m_device;
    delete m_surface;

    vkDestroyInstance(m_instance, nullptr);
}

/** Begin a new frame. */
void VulkanGPUManager::startFrame() {
    /* Begin a new frame and allocate the primary command buffer. */
    m_device->commandPool()->startFrame();
    m_primaryCmdBuf = m_device->commandPool()->allocateTransient();
    m_primaryCmdBuf->begin();

    /* Acquire a new image from the swap chain. */
    m_swapchain->startFrame();

    // TODO: Need to wait for present complete semaphore, transition layout.
}

/** End a frame and present it on screen. */
void VulkanGPUManager::endFrame() {
    // TODO: Transition image layout to present.

    m_primaryCmdBuf->end();

    m_swapchain->endFrame();
}
