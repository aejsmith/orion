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
 * @brief               Core Vulkan definitions.
 */

#pragma once

/** Whether to enable the Vulkan validation layers. */
#ifdef ORION_BUILD_DEBUG
#   define ORION_VULKAN_VALIDATION 1
#else
#   define ORION_VULKAN_VALIDATION 0
#endif

#include "loader.h"

#include "gpu/gpu_manager.h"

/** Macro to check the result of Vulkan API calls. */
#define checkVk(call) \
    { \
        VkResult __result = call; \
        if (unlikely(__result != VK_SUCCESS)) \
            fatal("Vulkan call failed: %d", __result); \
    }

class VulkanGPUManager;

/** Base class for a Vulkan child object. */
class VulkanObject {
public:
    /** @return             Manager that owns the object. */
    VulkanGPUManager *manager() const { return m_manager; }
protected:
    /** Initialise the object.
     * @param manager       Manager that owns the object. */
    explicit VulkanObject(VulkanGPUManager *manager) :
        m_manager(manager)
    {}
private:
    VulkanGPUManager *m_manager;            /**< Manager that owns the object. */
};

/** Base class for a Vulkan object that owns a handle. */
template <typename T>
class VulkanHandle : public VulkanObject {
public:
    /** @return             Handle to the object. */
    T handle() const { return m_handle; }
protected:
    /** Initialise the object.
     * @param manager       Manager that owns the object. */
    explicit VulkanHandle(VulkanGPUManager *manager) :
        VulkanObject(manager),
        m_handle(VK_NULL_HANDLE)
    {}

    T m_handle;                             /**< Handle to the object. */
};
