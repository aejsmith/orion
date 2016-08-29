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
 * @brief               Vulkan function pointers.
 */

#pragma once

#include <vulkan/vulkan.h>

#if ORION_VULKAN_VALIDATION
    #define ENUMERATE_VK_INSTANCE_DEBUG_FUNCTIONS(macro, features) \
        macro(CreateDebugReportCallbackEXT, features.validation) \
        macro(DestroyDebugReportCallbackEXT, features.validation)
#else
    #define ENUMERATE_VK_INSTANCE_DEBUG_FUNCTIONS(macro, features)
#endif

#define ENUMERATE_VK_INSTANCE_FUNCTIONS(macro, features) \
    ENUMERATE_VK_INSTANCE_DEBUG_FUNCTIONS(macro, features)

struct VulkanFeatures;

/** Structure containing instance extension function pointers. */
struct VulkanInstanceFunctions {
    #define DECLARE_VK_FUNCTION(name, cond) PFN_vk##name name;
    ENUMERATE_VK_INSTANCE_FUNCTIONS(DECLARE_VK_FUNCTION, bad);
    #undef DECLARE_VK_FUNCTION

    void init(VkInstance instance, VulkanFeatures &features);
};