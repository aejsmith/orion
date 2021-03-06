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
 * @brief               Vulkan queue class.
 */

#pragma once

#include "command_buffer.h"

class VulkanDevice;

/** Class managing a Vulkan queue. */
class VulkanQueue : public VulkanHandle<VkQueue> {
public:
    VulkanQueue(VulkanGPUManager *manager, uint32_t queueFamily, uint32_t index);

    void submit(VulkanCommandBuffer *cmdBuf,
                VulkanSemaphore *wait = nullptr,
                VkPipelineStageFlags waitStages = 0,
                VulkanSemaphore *signal = nullptr,
                VulkanFence *fence = nullptr);
};
