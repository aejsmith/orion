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

#include "manager.h"
#include "render_pass.h"

/** Initialise the render pass.
 * @param manager       Manager that owns this render pass.
 * @param desc          Descriptor for the render pass. */
VulkanRenderPass::VulkanRenderPass(VulkanGPUManager *manager, GPURenderPassDesc &&desc) :
    GPURenderPass(std::move(desc)),
    VulkanHandle(manager)
{
    std::vector<VkAttachmentDescription> attachments;
    attachments.reserve(m_desc.colourAttachments.size() + ((m_desc.depthStencilAttachment) ? 1 : 0));
    std::vector<VkAttachmentReference> attachmentRefs;
    attachmentRefs.reserve(attachments.capacity());

    auto convertLoadOp =
        [&] (GPURenderLoadOp inLoadOp) -> VkAttachmentLoadOp {
            switch (inLoadOp) {
                case GPURenderLoadOp::kLoad:
                    return VK_ATTACHMENT_LOAD_OP_LOAD;
                case GPURenderLoadOp::kClear:
                    return VK_ATTACHMENT_LOAD_OP_CLEAR;
                case GPURenderLoadOp::kDontCare:
                    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                default:
                    unreachable();
            }
        };

    auto addAttachment =
        [&] (const GPURenderAttachmentDesc &inAttachment, bool depthStencil) {
            attachments.emplace_back();
            auto &attachment = attachments.back();

            auto &format = manager->features().formats[inAttachment.format];
            attachment.format = format.format;

            if (depthStencil) {
                checkMsg(
                    format.properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    "Format does not support use as depth/stencil attachment");
            } else {
                checkMsg(
                    format.properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,
                    "Format does not support use as colour attachment");
            }

            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = convertLoadOp(inAttachment.loadOp);
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = convertLoadOp(inAttachment.stencilLoadOp);
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

            if (depthStencil) {
                attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            } else {
                attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

            attachmentRefs.emplace_back();
            auto &attachmentRef = attachmentRefs.back();
            attachmentRef.attachment = attachments.size() - 1;
            attachmentRef.layout = attachment.initialLayout;
        };

    for (auto &inAttachment : m_desc.colourAttachments)
        addAttachment(inAttachment, false);
    if (m_desc.depthStencilAttachment)
        addAttachment(m_desc.depthStencilAttachment, true);

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = m_desc.colourAttachments.size();
    subpass.pColorAttachments = &attachmentRefs[0];
    subpass.pDepthStencilAttachment = (m_desc.depthStencilAttachment)
        ? &attachmentRefs[attachmentRefs.size() - 1]
        : nullptr;

    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = attachments.size();
    createInfo.pAttachments = &attachments[0];
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;

    checkVk(vkCreateRenderPass(
        manager->device()->handle(),
        &createInfo,
        nullptr,
        &m_handle));
}

/** Destroy the render pass. */
VulkanRenderPass::~VulkanRenderPass() {
    vkDestroyRenderPass(manager()->device()->handle(), m_handle, nullptr);
}

/** Create a render pass object.
 * @param desc          Descriptor for the render pass.
 * @return              Created render pass object. */
GPURenderPassPtr VulkanGPUManager::createRenderPass(GPURenderPassDesc &&desc) {
    return new VulkanRenderPass(this, std::move(desc));
}

/** Create a framebuffer object.
 * @param targets       Render target descriptor.
 * @param pass          Pass that the framebuffer must be compatible with. */
VulkanFramebuffer::VulkanFramebuffer(
    VulkanGPUManager *manager,
    const GPURenderTargetDesc &targets,
    const VulkanRenderPass *pass)
    :
    VulkanHandle(manager),
    m_targets(targets)
{
    /* Create image views. */
    m_views.reserve(targets.colour.size() + ((targets.depthStencil) ? 1 : 0));

    uint32_t width = 0;
    uint32_t height = 0;

    auto createView =
        [&] (const GPUTextureImageRef &imageRef) {
            VkImageViewCreateInfo viewCreateInfo = {};
            viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

            if (imageRef) {
                auto texture = static_cast<VulkanTexture *>(imageRef.texture);

                viewCreateInfo.image = texture->handle();
                viewCreateInfo.format = manager->features().formats[texture->format()].format;
                viewCreateInfo.subresourceRange.aspectMask = VulkanUtil::aspectMaskForFormat(texture->format());

                /* These are validated to be the same for each texture. */
                width = texture->width();
                height = texture->height();
            } else {
                VulkanSurface *surface = manager->surface();

                /* We assume that when we're creating a new framebuffer for the
                 * main window we want it to refer to current swapchain image.
                 * The swapchain image forms part of the framebuffer key, so
                 * this framebuffer will only be used for this specific image. */
                viewCreateInfo.image = manager->swapchain()->currentImage();
                viewCreateInfo.format = surface->surfaceFormat().format;
                viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

                width = surface->width();
                height = surface->height();
            }

            /* We're rendering to a single layer. */
            viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

            viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
            viewCreateInfo.subresourceRange.baseMipLevel = imageRef.mip;
            viewCreateInfo.subresourceRange.levelCount = 1;
            viewCreateInfo.subresourceRange.baseArrayLayer = imageRef.layer;
            viewCreateInfo.subresourceRange.layerCount = 1;

            VkImageView view;
            checkVk(vkCreateImageView(manager->device()->handle(), &viewCreateInfo, nullptr, &view));
            m_views.push_back(view);
        };

    /* This must follow the order used in VulkanRenderPass' constructor. */
    for (auto &imageRef : targets.colour)
        createView(imageRef);
    if (targets.depthStencil)
        createView(targets.depthStencil);

    VkFramebufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = pass->handle();
    createInfo.attachmentCount = m_views.size();
    createInfo.pAttachments = &m_views[0];
    createInfo.width = width;
    createInfo.height = height;
    createInfo.layers = 1;

    checkVk(vkCreateFramebuffer(manager->device()->handle(), &createInfo, nullptr, &m_handle));
}

/** Destroy the framebuffer. */
VulkanFramebuffer::~VulkanFramebuffer() {
    for (VkImageView view : m_views)
        vkDestroyImageView(manager()->device()->handle(), view, nullptr);

    vkDestroyFramebuffer(manager()->device()->handle(), m_handle, nullptr);
}

/** Invalidate framebuffers associated with a texture.
 * @param texture       Texture being destroyed. */
void VulkanGPUManager::invalidateFramebuffers(const VulkanTexture *texture) {
    for (auto it = m_framebuffers.begin(); it != m_framebuffers.end();) {
        const GPURenderTargetDesc &targets = it->first.targets;
        bool invalidate = false;

        if (targets.depthStencil.texture == texture) {
            invalidate = true;
        } else {
            for (size_t i = 0; i < targets.colour.size(); i++) {
                if (targets.colour[i].texture == texture)
                    invalidate = true;
            }
        }

        if (invalidate) {
            delete it->second;
            it = m_framebuffers.erase(it);
        } else {
            ++it;
        }
    }
}

/** Transition an image to the appropriate layout before/after a pass.
 * @param cmdBuf        Command buffer to use.
 * @param imageRef      Image reference.
 * @param begin         Whether this is the beginning of the pass. */
static void transitionRenderTarget(
    VulkanCommandBuffer *cmdBuf,
    VulkanSwapchain *swapchain,
    const GPUTextureImageRef &imageRef,
    bool begin)
{
    VkImageSubresourceRange subresources;
    subresources.baseMipLevel = imageRef.mip;
    subresources.levelCount = 1;
    subresources.baseArrayLayer = imageRef.layer;
    subresources.layerCount = 1;

    VkImage handle;
    bool isDepth;
    if (imageRef) {
        auto texture = static_cast<VulkanTexture *>(imageRef.texture);
        handle = texture->handle();
        isDepth = PixelFormat::isDepth(texture->format());
        subresources.aspectMask = VulkanUtil::aspectMaskForFormat(texture->format());
    } else {
        handle = swapchain->currentImage();
        isDepth = false;
        subresources.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageLayout defaultLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkImageLayout attachmentLayout = (isDepth)
        ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VulkanUtil::setImageLayout(
        cmdBuf,
        handle,
        subresources,
        (begin) ? defaultLayout : attachmentLayout,
        (begin) ? attachmentLayout : defaultLayout);

    if (begin && imageRef) {
        /* Reference the textures in the command buffer. */
        cmdBuf->addReference(imageRef.texture);
    }
}

/** Begin a render pass.
 * @param desc          Descriptor for the render pass instance. */
void VulkanGPUManager::beginRenderPass(const GPURenderPassInstanceDesc &desc) {
    VulkanFrame &frame = currentFrame();

    check(!frame.renderPass);

    auto pass = static_cast<const VulkanRenderPass *>(desc.pass);

    /* Validate render pass state. */
    pass->validateInstance(desc);

    /* Look for an existing suitable framebuffer. For the main window, we must
     * key the framebuffer to the current swapchain image. */
    VulkanFramebuffer *framebuffer;
    VulkanFramebufferKey key(
        desc.targets,
        pass,
        (desc.targets.isMainWindow()) ? m_swapchain->currentImage() : VK_NULL_HANDLE);
    auto ret = m_framebuffers.find(key);
    if (ret == m_framebuffers.end()) {
        /* Create a new one. */
        framebuffer = new VulkanFramebuffer(this, desc.targets, pass);
        m_framebuffers.emplace(std::move(key), framebuffer);
    } else {
        framebuffer = ret->second;
    }

    /* Transition the images to the right layout. */
    for (auto &imageRef : desc.targets.colour)
        transitionRenderTarget(frame.primaryCmdBuf, m_swapchain, imageRef, true);
    if (desc.targets.depthStencil)
        transitionRenderTarget(frame.primaryCmdBuf, m_swapchain, desc.targets.depthStencil, true);

    /* Prepare clear values. */
    std::vector<VkClearValue> clearValues;
    clearValues.reserve(desc.targets.colour.size() + ((desc.targets.depthStencil) ? 1 : 0));

    for (auto &clearColour : desc.clearColours) {
        VkClearValue clearValue;
        clearValue.color.float32[0] = clearColour.r;
        clearValue.color.float32[1] = clearColour.g;
        clearValue.color.float32[2] = clearColour.b;
        clearValue.color.float32[3] = clearColour.a;
        clearValues.push_back(clearValue);
    }

    if (desc.targets.depthStencil) {
        VkClearValue clearValue;
        clearValue.depthStencil.depth = desc.clearDepth;
        clearValue.depthStencil.stencil = desc.clearStencil;
        clearValues.push_back(clearValue);
    }

    /* Begin the pass. */
    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = pass->handle();
    beginInfo.framebuffer = framebuffer->handle();
    /* TODO: "There may be a performance cost for using a render area smaller
     * than the framebuffer, unless it matches the render area granularity for
     * the render pass". */
    beginInfo.renderArea.offset.x = desc.renderArea.x;
    beginInfo.renderArea.offset.y = desc.renderArea.y;
    beginInfo.renderArea.extent.width = desc.renderArea.width;
    beginInfo.renderArea.extent.height = desc.renderArea.height;
    beginInfo.clearValueCount = clearValues.size();
    beginInfo.pClearValues = &clearValues[0];

    vkCmdBeginRenderPass(frame.primaryCmdBuf->handle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    frame.renderPass = pass;
    frame.framebuffer = framebuffer;

    /* Set up default state for the pass. */
    setViewport(desc.renderArea);
    setScissor(false, IntRect());
    GPUManager::setBlendState<>();
    GPUManager::setDepthStencilState<>();
    GPUManager::setRasterizerState<>();
}

/** End the current render pass. */
void VulkanGPUManager::endRenderPass() {
    VulkanFrame &frame = currentFrame();
    check(frame.renderPass);

    /* End the pass. */
    vkCmdEndRenderPass(frame.primaryCmdBuf->handle());

    /* Transition the images back from their attachment layouts. */
    const GPURenderTargetDesc &targets = frame.framebuffer->targets();
    for (auto &imageRef : targets.colour)
        transitionRenderTarget(frame.primaryCmdBuf, m_swapchain, imageRef, false);
    if (targets.depthStencil)
        transitionRenderTarget(frame.primaryCmdBuf, m_swapchain, targets.depthStencil, false);

    frame.renderPass = nullptr;
}
