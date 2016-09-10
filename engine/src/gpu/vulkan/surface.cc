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
 * @brief               Vulkan surface class.
 */

#include <SDL.h>

#ifdef SDL_VIDEO_DRIVER_X11
    #define VK_USE_PLATFORM_XCB_KHR 1

    /* Avoid conflicts with our own Window type. */
    #define Window X11Window

    #include <X11/Xlib-xcb.h>
#endif

#include <SDL_syswm.h>

#ifdef SDL_VIDEO_DRIVER_X11
    #undef Window
#endif

#include "device.h"
#include "surface.h"

#include "engine/window.h"

/** Create the window.
 * @param manager       Manager that owns the surface.
 * @param config        Engine configuration structure. */
VulkanSurface::VulkanSurface(VulkanGPUManager *manager, const EngineConfiguration &config) :
    Window(config),
    VulkanHandle(manager)
{}

/** Initialise the surface. */
void VulkanSurface::init() {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(m_sdlWindow, &wmInfo))
        fatal("Failed to get SDL WM info: %s", SDL_GetError());

    VkResult result;

    switch (wmInfo.subsystem) {
        #ifdef SDL_VIDEO_DRIVER_X11
            case SDL_SYSWM_X11:
            {
                VkXcbSurfaceCreateInfoKHR createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
                createInfo.connection = XGetXCBConnection(wmInfo.info.x11.display);
                createInfo.window = wmInfo.info.x11.window;

                result = vkCreateXcbSurfaceKHR(manager()->instance(), &createInfo, nullptr, &m_handle);
                break;
            }
        #endif

        default:
            unreachable();
    }

    if (result != VK_SUCCESS)
        fatal("Failed to create Vulkan surface: %d", result);
}

/** Choose the surface format to use based on the chosen physical device.
 * @param device        Created Vulkan device.
 * @param features      Feature information. */
void VulkanSurface::chooseFormat(VulkanDevice *device, const VulkanFeatures &features) {
    VkResult result;
    uint32_t count;

    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device->physicalHandle(), m_handle, &count, nullptr);
    if (result != VK_SUCCESS) {
        fatal("Failed to get Vulkan surface formats (1): %d", result);
    } else if (count == 0) {
        fatal("No Vulkan surface formats");
    }

    std::vector<VkSurfaceFormatKHR> formats(count);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device->physicalHandle(), m_handle, &count, &formats[0]);
    if (result != VK_SUCCESS)
        fatal("Failed to get Vulkan surface formats (2): %d", result);

    m_surfaceFormat.colorSpace = formats[0].colorSpace;

    /* A single entry with undefined format means that there is no preferred
     * format, and we can choose whatever we like. */
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        m_surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    } else {
        /* Default to the first format in the list. */
        m_surfaceFormat.format = formats[0].format;

        /* Search for our desired format (R8G8B8A8 unsigned normalised). */
        for (const VkSurfaceFormatKHR &format : formats) {
            if (format.format == VK_FORMAT_R8G8B8A8_UNORM)
                m_surfaceFormat.format = format.format;
        }
    }

    /* Now we need to convert this back to a generic pixel format definition. */
    for (int i = 0; i < PixelFormat::kNumFormats; i++) {
        if (features.formats[i].format == m_surfaceFormat.format) {
            m_format = static_cast<PixelFormat::Impl>(i);
            return;
        }
    }

    fatal("Could not match Vulkan surface format to PixelFormat");
}

/** Destroy the surface. */
void VulkanSurface::destroy() {
    vkDestroySurfaceKHR(manager()->instance(), m_handle, nullptr);
}

/** Get the platform-specific surface extension name.
 * @return              Extension name. */
const char *VulkanSurface::getPlatformExtensionName() {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(m_sdlWindow, &wmInfo))
        fatal("Failed to get SDL WM info: %s", SDL_GetError());

    switch (wmInfo.subsystem) {
        #ifdef SDL_VIDEO_DRIVER_X11
            case SDL_SYSWM_X11:
                return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
        #endif

        default:
            fatal("SDL video subsystem %d is not supported", wmInfo.subsystem);
    }
}
