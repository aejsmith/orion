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
 * @brief               Vulkan texture implementation.
 */

#pragma once

#include "memory_manager.h"

/** Vulkan texture implementation. */
class VulkanTexture :
    public GPUTexture,
    public VulkanHandle<VkImage> {
public:
    VulkanTexture(VulkanGPUManager *manager, const GPUTextureDesc &desc);
    VulkanTexture(VulkanGPUManager *manager, const GPUTextureImageRef &image);

    ~VulkanTexture();

    void update(const IntRect &area, const void *data, unsigned mip, unsigned layer) override;
    void update(const IntBox &area, const void *data, unsigned mip) override;
    void generateMipmap() override;

    /** @return             Memory allocation backing this image. */
    VulkanMemoryManager::ImageMemory *allocation() const { return m_allocation; }
private:
    /** Memory allocation backing this image. */
    VulkanMemoryManager::ImageMemory *m_allocation;

    bool m_new;                     /**< Whether this texture is newly created. */
};

/** Vulkan sampler state object implementation. */
class VulkanSamplerState :
    public GPUSamplerState,
    public VulkanHandle<VkSampler> {
public:
    VulkanSamplerState(VulkanGPUManager *manager, const GPUSamplerStateDesc &desc);
    ~VulkanSamplerState();
};
