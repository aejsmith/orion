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
 * Vulkan render pass compatibility key.
 *
 * This key is used to determine whether render passes are compatible with each
 * other. This is used for framebuffers and pipeline objects, which are created
 * with a specific render pass but can be used with any that are compatible
 * with it.
 *
 * For now we only have to worry about attachment count and formats because we
 * don't use subpasses. This will need to be updated if we do in future.
 */
struct VulkanRenderPassCompatibilityKey {
    /** Structure describing an attachment. */
    struct Attachment {
        PixelFormat format;             /**< Pixel format of the attachment. */
        // TODO: Sample count.
    };

    /** Array of colour attachments. */
    std::vector<Attachment> colourAttachments;

    /** Depth/stencil attachment. */
    Attachment depthStencilAttachment;

    /** Initialise the key from a render pass.
     * @param pass          Pass to initialise from. */
    explicit VulkanRenderPassCompatibilityKey(const VulkanRenderPass *pass) {
        const auto &desc = pass->desc();

        colourAttachments.reserve(desc.colourAttachments.size());
        for (const auto &attachment : desc.colourAttachments) {
            colourAttachments.emplace_back();
            colourAttachments.back().format = attachment.format;
        }

        depthStencilAttachment.format = desc.depthStencilAttachment.format;
    }

    /** Compare this key with another. */
    bool operator ==(const VulkanRenderPassCompatibilityKey &other) const {
        if (colourAttachments.size() != other.colourAttachments.size())
            return false;

        for (size_t i = 0; i < colourAttachments.size(); i++) {
            if (colourAttachments[i].format != other.colourAttachments[i].format)
                return false;
        }

        if (depthStencilAttachment.format != other.depthStencilAttachment.format)
            return false;

        return true;
    }

    /** Get a hash from a render pass compatibility key. */
    friend size_t hashValue(const VulkanRenderPassCompatibilityKey &key) {
        size_t hash = hashValue(key.colourAttachments.size());

        for (size_t i = 0; i < key.colourAttachments.size(); i++)
            hash = hashCombine(hash, key.colourAttachments[i].format);

        hash = hashCombine(hash, key.depthStencilAttachment.format);

        return hash;
    }
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

    /** Render pass compatibility key. */
    VulkanRenderPassCompatibilityKey renderPass;

    /**
     * Swapchain image handle.
     *
     * For a framebuffer that refers to the main window, we cannot rely on just
     * the RT descriptor because we have multiple swapchain images and we need
     * a different framebuffer for each one. Therefore if the RT is the main
     * window, we set this to the swapchain image handle the framebuffer was
     * created for. If the RT is not the main window, this is set to null.
     */
    VkImage swapchainImage;

    /** Initialise the key.
     * @param inTargets     Render target descriptor.
     * @param inPass        Pass that the framebuffer should be compatible with.
     * @param inSwapImage   For the main window, swapchain image this framebuffer
     *                      is for. */
    VulkanFramebufferKey(
        const GPURenderTargetDesc &inTargets,
        const VulkanRenderPass *inPass,
        VkImage inSwapImage)
        :
        targets(inTargets),
        renderPass(inPass),
        swapchainImage(inSwapImage)
    {}

    /** Compare this key with another. */
    bool operator ==(const VulkanFramebufferKey &other) const {
        return
            targets == other.targets &&
            renderPass == other.renderPass &&
            swapchainImage == other.swapchainImage;
    }

    /** Get a hash from a framebuffer key. */
    friend size_t hashValue(const VulkanFramebufferKey &key) {
        size_t hash = hashValue(key.targets);
        hash = hashCombine(hash, key.renderPass);
        hash = hashCombine(hash, key.swapchainImage);
        return hash;
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
