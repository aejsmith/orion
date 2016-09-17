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
 * @brief               Vulkan render pass implementation.
 */

#pragma once

#include "vulkan.h"

/** Vulkan render pass class. */
class VulkanRenderPass :
    public GPURenderPass,
    public VulkanHandle<VkRenderPass> {
public:
    VulkanRenderPass(VulkanGPUManager *manager, GPURenderPassDesc &&desc);
    ~VulkanRenderPass();
};

/**
 * Key for a Vulkan framebuffer.
 *
 * This is a key to find framebuffers with given render targets that are
 * compatible with a given render pass. See the Vulkan spec on rules for
 * render pass compatibility.
 */
struct VulkanFramebufferKey {
    /** Render targets. */
    GPURenderTargetDesc targets;

    /** Initialise the key.
     * @param inTargets     Render target descriptor.
     * @param inPass        Pass that the framebuffer should be compatible with. */
    VulkanFramebufferKey(const GPURenderTargetDesc &inTargets, const VulkanRenderPass *inPass) :
        targets(inTargets)
    {
        /* Nothing else to do here for now, since we only support a single
         * subpass at the moment. This means all that matters is the format
         * and sample count of the attachments, which is covered by equality of
         * the render target descriptors. If we add multiple subpass support,
         * we will need to handle that here. */
    }

    /** Compare this key with another. */
    bool operator ==(const VulkanFramebufferKey &other) const {
        return targets == other.targets;
    }

    /** Get a hash from a framebuffer key. */
    friend size_t hashValue(const VulkanFramebufferKey &key) {
        return hashValue(key.targets);
    }
};

/** Vulkan framebuffer class. */
class VulkanFramebuffer : public VulkanHandle<VkFramebuffer> {
public:
    VulkanFramebuffer(
        VulkanGPUManager *manager,
        const GPURenderTargetDesc &targets,
        const VulkanRenderPass *pass);
    ~VulkanFramebuffer();

    /** @return             Render target descriptor. */
    const GPURenderTargetDesc &targets() const { return m_targets; }
private:
    /** Render target descriptor. */
    GPURenderTargetDesc m_targets;
    /** Array of views for each attachment. */
    std::vector<VkImageView> m_views;
};
