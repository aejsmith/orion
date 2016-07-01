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

#include "surface.h"

#include "engine/window.h"

/** Get the platform-specific surface extension name.
 * @param window        Main window.
 * @return              Extension name. */
const char *VulkanSurface::getPlatformExtensionName(Window *window) {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(window->sdlWindow(), &wmInfo))
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

/** Create a Vulkan surface for a window.
 * @param window        Window to create for. */
VulkanSurface::VulkanSurface(Window *window) :
    m_window(window)
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(m_window->sdlWindow(), &wmInfo))
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

                result = vkCreateXcbSurfaceKHR(g_vulkan->instance(), &createInfo, nullptr, &m_handle);
                break;
            }
        #endif

        default:
            unreachable();
    }

    if (result != VK_SUCCESS)
        fatal("Failed to create Vulkan surface: %d", result);
}

/** Destroy the surface. */
VulkanSurface::~VulkanSurface() {
    vkDestroySurfaceKHR(g_vulkan->instance(), m_handle, nullptr);
}
