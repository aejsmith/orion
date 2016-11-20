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

#include "manager.h"

/** Get all required instance function pointers.
 * @param instance      Vulkan instance.
 * @param features      Features structure detailing enabled extensions. */
void VulkanInstanceFunctions::init(VkInstance instance, const VulkanFeatures &features) {
    #define GET_VK_FUNCTION(name, cond) \
        if (cond) { \
            name = reinterpret_cast<PFN_vk##name>(vkGetInstanceProcAddr(instance, "vk" #name)); \
            if (!name) \
                fatal("Vulkan instance function 'vk" #name "' not found"); \
        }

    ENUMERATE_VK_INSTANCE_FUNCTIONS(GET_VK_FUNCTION, features);

    #undef GET_VK_FUNCTION
}

/** Get all required device function pointers.
 * @param device        Vulkan device handle.
 * @param features      Features structure detailing enabled extensions. */
void VulkanDeviceFunctions::init(VkDevice device, const VulkanFeatures &features) {
    #define GET_VK_FUNCTION(name, cond) \
        if (cond) { \
            name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(device, "vk" #name)); \
            if (!name) \
                fatal("Vulkan instance function 'vk" #name "' not found"); \
        }

    ENUMERATE_VK_DEVICE_FUNCTIONS(GET_VK_FUNCTION, features);

    #undef GET_VK_FUNCTION
}
