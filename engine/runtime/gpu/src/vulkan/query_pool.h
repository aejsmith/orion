/*
 * Copyright (C) 2017 Alex Smith
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
 * @brief               Vulkan query pool class.
 */

#pragma once

#include "vulkan.h"

#include "gpu/query_pool.h"

class VulkanCommandBuffer;

/** Vulkan implementation of GPUQueryPool. */
class VulkanQueryPool : public GPUQueryPool,
                        public VulkanHandle<VkQueryPool> {
public:
    VulkanQueryPool(VulkanGPUManager *manager,
                    const GPUQueryPoolDesc &desc);
    ~VulkanQueryPool();

    void reset(uint32_t start, uint32_t count) override;
    void getResults(uint32_t start,
                    uint32_t count,
                    uint64_t *data,
                    bool flush) override;

    void end(uint32_t index, VulkanCommandBuffer *cmdBuf);
};
