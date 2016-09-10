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
 * @brief               Vulkan resource set implementation.
 */

#pragma once

#include "vulkan.h"

/** Vulkan resource set layout implementation. */
class VulkanResourceSetLayout : public GPUResourceSetLayout {
public:
    explicit VulkanResourceSetLayout(GPUResourceSetLayoutDesc &&desc);
    ~VulkanResourceSetLayout();
private:
    VkDescriptorSetLayout m_handle;     /**< Handle to the layout. */
};

/** Vulkan resource set implementation. */
class VulkanResourceSet : public GPUResourceSet {
public:
    explicit VulkanResourceSet(GPUResourceSetLayout *layout);
    ~VulkanResourceSet();
protected:
    void updateSlot(size_t index) override;
};
