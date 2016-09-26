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
class VulkanRenderPass : public GPURenderPass, public VulkanHandle<VkRenderPass> {
public:
    VulkanRenderPass(VulkanGPUManager *manager, GPURenderPassDesc &&desc);
protected:
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

    /** Initialise the key.
     * @param inTargets     Render target descriptor.
     * @param inPass        Pass that the framebuffer should be compatible with. */
    VulkanFramebufferKey(const GPURenderTargetDesc &inTargets, const VulkanRenderPass *inPass) :
        targets(inTargets),
        renderPass(inPass)
    {}

    /** Compare this key with another. */
    bool operator ==(const VulkanFramebufferKey &other) const {
        return
            targets == other.targets &&
            renderPass == other.renderPass;
    }

    /** Get a hash from a framebuffer key. */
    friend size_t hashValue(const VulkanFramebufferKey &key) {
        size_t hash = hashValue(key.targets);
        hash = hashCombine(hash, key.renderPass);
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

    /** @return             Current target size. */
    glm::ivec2 size() const {
        const GPUTexture *texture = (m_targets.colour.size())
            ? m_targets.colour[0].texture
            : m_targets.depthStencil.texture;
        return glm::ivec2(texture->width(), texture->height());
    }
private:
    /** Render target descriptor. */
    GPURenderTargetDesc m_targets;
    /** Array of views for each attachment. */
    std::vector<VkImageView> m_views;
};
