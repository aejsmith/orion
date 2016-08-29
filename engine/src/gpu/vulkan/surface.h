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

#pragma once

#include "vulkan.h"

#include "engine/window.h"

struct VulkanFeatures;

/**
 * Class wrapping a Vulkan surface.
 *
 * This class is an extension of the generic Window class which encapsulates
 * all of the platform specifics of getting a Vulkan surface object referring
 * to the application window. It uses the required platform-specific extensions
 * to create a surface, which can then be used by the platform independent code.
 */
class VulkanSurface : public Window {
public:
    explicit VulkanSurface(const EngineConfiguration &config);

    void init();
    void chooseFormat(VulkanDevice *device, const VulkanFeatures &features);
    void destroy();

    const char *getPlatformExtensionName();

    /** @return             Handle to the surface. */
    VkSurfaceKHR handle() const { return m_handle; }
    /** @return             Vulkan surface format. */
    VkSurfaceFormatKHR surfaceFormat() const { return m_surfaceFormat; }
private:
    VkSurfaceKHR m_handle;              /**< Handle to the surface. */
    VkSurfaceFormatKHR m_surfaceFormat; /**< Vulkan surface format. */
};

