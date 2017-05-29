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

#include "manager.h"

#include "core/hash_table.h"
#include "core/string.h"

#include "engine/engine.h"

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
 * @param surface       Surface for main window.
 * @param layers        Where to store array of layers to enable.
 * @param extensions    Where to store array of extensions to enable.
 * @param features      Features structure to fill in. */
static void enableInstanceExtensions(
    VulkanSurface *surface,
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
    extensions.push_back(surface->getPlatformExtensionName());
    for (const char *extension : extensions) {
        if (availableExtensions.find(extension) == availableExtensions.end())
            fatal("Required Vulkan instance extension '%s' not available", extension);
    }

    /* Enable validation extensions if requested and present. */
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

/** Filters on debug messages. */
static const char *kDebugMessageFilters[] = {
    /* Can't completely eliminate this, and it spams a lot. */
    "any subsequent sets were disturbed by newly bound pipelineLayout",
};

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
    for (const char *filter : kDebugMessageFilters) {
        if (std::strstr(pMessage, filter))
            return VK_FALSE;
    }

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
    m_features()
{
    VkResult result;

    /* Create the main window. We do this first as we need it to get the surface
     * extension that we need to enable. We do not yet initialise the surface. */
    m_surface = new VulkanSurface(this, config);
    window = m_surface;

    logInfo("Initialising Vulkan");

    /* Determine the extensions to use. */
    std::vector<const char *> enabledLayers;
    std::vector<const char *> enabledExtensions;
    enableInstanceExtensions(m_surface, enabledLayers, enabledExtensions, m_features);

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

    /* Get instance extension function pointers. */
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

        m_functions.CreateDebugReportCallbackEXT(
            m_instance, &callbackCreateInfo, nullptr, &m_debugReportCallback);
    #endif

    /* Now we can create the surface. */
    m_surface->create();

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
    std::vector<std::pair<std::unique_ptr<VulkanDevice>, VulkanFeatures>> devices(deviceCount);
    uint32_t bestDevice = UINT32_MAX;
    for (uint32_t i = 0; i < physicalDevices.size(); i++) {
        logInfo("  Device %u:", i);

        std::unique_ptr<VulkanDevice> device = std::make_unique<VulkanDevice>(this, physicalDevices[i]);
        devices[i].second = m_features;
        if (device->identify(m_surface, devices[i].second)) {
            if (bestDevice == UINT32_MAX || device->isBetterThan(devices[bestDevice].first.get()))
                bestDevice = i;

            devices[i].first = std::move(device);
        }
    }

    if (bestDevice == UINT32_MAX)
        fatal("No suitable Vulkan physical device found");

    logInfo("  Using device %u", bestDevice);
    m_device = devices[bestDevice].first.release();
    m_features = devices[bestDevice].second;
    devices.clear();

    /* Create the logical device. */
    m_device->init();

    /* Initialise other feature information. */
    initFeatures();

    /* Create other global objects. */
    m_queue = new VulkanQueue(this, m_device->queueFamily(), 0);
    m_commandPool = new VulkanCommandPool(this);
    m_descriptorPool = new VulkanDescriptorPool(this);
    m_memoryManager = new VulkanMemoryManager(this);

    /* Choose a surface format and create a swapchain. */
    m_surface->chooseFormat();
    m_swapchain = new VulkanSwapchain(this);

    /* Begin the first frame. */
    startFrame();

    /* Finally create our backing texture for the main window. This must be done
     * after beginning the first frame as it needs a staging command buffer when
     * setting up the texture. */
    m_surface->finalise();
}

/** Shut down the Vulkan GPU manager. */
VulkanGPUManager::~VulkanGPUManager() {
    /* Wait for the device to finish, and clean up all frames still in flight. */
    vkDeviceWaitIdle(m_device->handle());
    cleanupFrames(true);

    /* Delete all framebuffer objects. */
    invalidateFramebuffers(nullptr);

    /* Destroy all cached state objects. */
    destroyStates();

    delete m_swapchain;
    delete m_memoryManager;
    delete m_descriptorPool;
    delete m_commandPool;
    delete m_queue;

    /* Freed by the engine, but we need to destroy the surface prior to the
     * instance to avoid validation errors. */
    m_surface->destroy();

    delete m_device;

    #if ORION_VULKAN_VALIDATION
        /** Destroy the debug report callback. */
        m_functions.DestroyDebugReportCallbackEXT(m_instance, m_debugReportCallback, nullptr);
    #endif

    vkDestroyInstance(m_instance, nullptr);
}

/** Initialise the feature information table. */
void VulkanGPUManager::initFeatures() {
    /* Initialise the format mapping table and check for support. */
    auto initFormat =
        [&] (PixelFormat engineFormat, VkFormat vkFormat) {
            auto &format = m_features.formats[engineFormat];
            format.format = vkFormat;
            vkGetPhysicalDeviceFormatProperties(
                m_device->physicalHandle(),
                vkFormat,
                &format.properties);
            if (!format.properties.linearTilingFeatures &&
                !format.properties.optimalTilingFeatures &&
                !format.properties.bufferFeatures)
            {
                fatal("Required Vulkan image format %u (for %u) is not supported", vkFormat, engineFormat);
            }
        };
    initFormat(PixelFormat::kR8G8B8A8,          VK_FORMAT_R8G8B8A8_UNORM);
    initFormat(PixelFormat::kR8G8B8A8sRGB,      VK_FORMAT_R8G8B8A8_SRGB);
    initFormat(PixelFormat::kR8G8B8,            VK_FORMAT_R8G8B8_UNORM);
    initFormat(PixelFormat::kR8G8B8sRGB,        VK_FORMAT_R8G8B8_SRGB);
    initFormat(PixelFormat::kR8G8,              VK_FORMAT_R8G8_UNORM);
    initFormat(PixelFormat::kR8,                VK_FORMAT_R8_UNORM);
    initFormat(PixelFormat::kB8G8R8A8,          VK_FORMAT_B8G8R8A8_UNORM);
    initFormat(PixelFormat::kB8G8R8A8sRGB,      VK_FORMAT_B8G8R8A8_SRGB);
    initFormat(PixelFormat::kB8G8R8,            VK_FORMAT_B8G8R8_UNORM);
    initFormat(PixelFormat::kB8G8R8sRGB,        VK_FORMAT_B8G8R8_SRGB);
    initFormat(PixelFormat::kR10G10B10A2,       VK_FORMAT_A2B10G10R10_UNORM_PACK32);
    initFormat(PixelFormat::kFloatR16G16B16A16, VK_FORMAT_R16G16B16A16_SFLOAT);
    initFormat(PixelFormat::kFloatR16G16B16,    VK_FORMAT_R16G16B16_SFLOAT);
    initFormat(PixelFormat::kFloatR16G16,       VK_FORMAT_R16G16_SFLOAT);
    initFormat(PixelFormat::kFloatR16,          VK_FORMAT_R16_SFLOAT);
    initFormat(PixelFormat::kFloatR32G32B32A32, VK_FORMAT_R32G32B32A32_SFLOAT);
    initFormat(PixelFormat::kFloatR32G32B32,    VK_FORMAT_R32G32B32_SFLOAT);
    initFormat(PixelFormat::kFloatR32G32,       VK_FORMAT_R32G32_SFLOAT);
    initFormat(PixelFormat::kFloatR32,          VK_FORMAT_R32_SFLOAT);
    initFormat(PixelFormat::kDepth16,           VK_FORMAT_D16_UNORM);
    initFormat(PixelFormat::kDepth32,           VK_FORMAT_D32_SFLOAT);
    initFormat(PixelFormat::kDepth32Stencil8,   VK_FORMAT_D32_SFLOAT_S8_UINT);
}

/** Begin a new frame. */
void VulkanGPUManager::startFrame() {
    /* Start the new frame. */
    m_frames.emplace_back(this);
    VulkanFrame &frame = currentFrame();

    /* Allocate the primary command buffer. */
    frame.primaryCmdBuf = m_commandPool->allocateTransient(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    frame.primaryCmdBuf->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    /* Acquire a new image from the swap chain. */
    m_swapchain->startFrame();
}

/** Clean up completed frames.
 * @param shutdown      If true, the engine is being shut down so all frames
 *                      should have completed. */
void VulkanGPUManager::cleanupFrames(bool shutdown) {
    for (auto i = m_frames.begin(); i != m_frames.end(); ) {
        VulkanFrame &frame = *i;

        /* Check whether the frame has completed. We're about to start a new
         * frame, so if the current frame count is on the limit of how many we
         * can have pending, we must wait for the oldest one to finish. */
        bool completed = shutdown;
        if (!completed) {
            if (m_frames.size() >= kNumPendingFrames) {
                frame.fence.wait();
                completed = true;
            } else {
                completed = frame.fence.getStatus();
            }
        }

        /* Perform cleanup work on the frame. */
        m_commandPool->cleanupFrame(frame, completed);
        m_memoryManager->cleanupFrame(frame, completed);

        /* Remove the frame if it has completed. */
        if (completed) {
            m_frames.erase(i++);
        } else {
            ++i;
        }
    }
}

/** End a frame and present it on screen. */
void VulkanGPUManager::endFrame() {
    VulkanFrame &completedFrame = currentFrame();

    /* Perform any host to device transfers pending. */
    m_memoryManager->flushStagingCmdBuf();

    /* Submit and present the frame. */
    m_swapchain->endFrame(completedFrame.primaryCmdBuf, &completedFrame.fence);

    /* Clean up completed frames and wait for pending frames. */
    cleanupFrames(false);

    /* Prepare state for the next frame. */
    startFrame();
}
