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

#include "device.h"
#include "render_pass.h"

/** Initialise the render pass.
 * @param desc          Descriptor for the render pass. */
VulkanRenderPass::VulkanRenderPass(GPURenderPassDesc &&desc) :
    GPURenderPass(std::move(desc))
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
                    check(false);
                    unreachable();
            }
        };

    auto addAttachment =
        [&] (const GPURenderAttachmentDesc &inAttachment, bool depthStencil) {
            attachments.emplace_back();
            auto &attachment = attachments.back();

            auto &format = g_vulkan->features().formats[inAttachment.format];
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
        g_vulkan->device()->handle(),
        &createInfo,
        nullptr,
        &m_handle));
}

/** Destroy the render pass. */
VulkanRenderPass::~VulkanRenderPass() {
    vkDestroyRenderPass(g_vulkan->device()->handle(), m_handle, nullptr);
}

/** Create a render pass object.
 * @param desc          Descriptor for the render pass.
 * @return              Created render pass object. */
GPURenderPassPtr VulkanGPUManager::createRenderPass(GPURenderPassDesc &&desc) {
    return new VulkanRenderPass(std::move(desc));
}
