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
 * @brief               GPU render pass definitions.
 */

#include "engine/window.h"

#include "gpu/gpu_manager.h"
#include "gpu/render_pass.h"

/** Initialise a render pass object.
 * @param desc          Descriptor for the render pass. */
GPURenderPass::GPURenderPass(GPURenderPassDesc &&desc) :
    GPUState (std::move(desc))
{
    check(m_desc.colourAttachments.size() <= kMaxColourRenderTargets);
    check(m_desc.colourAttachments.size() > 0 || m_desc.depthStencilAttachment);

    for (const GPURenderAttachmentDesc &attachment : m_desc.colourAttachments) {
        /* Note: This is assumed in the API-specific implementations as well.
         * If this ever changes (i.e. we allow sparse attachment indices), those
         * will need to be updated accordingly. */
        check(attachment);

        check(PixelFormat::isColour(attachment.format));
    }

    if (m_desc.depthStencilAttachment)
        check(PixelFormat::isDepth(m_desc.depthStencilAttachment.format));
}

/** Validate a render pass instance.
 * @param instanceDesc  Render pass instance descriptor.
 * @return              Created instance. */
GPURenderPassInstance *GPURenderPass::createInstance(const GPURenderPassInstanceDesc &instanceDesc) const {
    /* Validate the instance. */
    #ifdef ORION_BUILD_DEBUG
        uint32_t width = 0;
        uint32_t height = 0;

        const GPURenderTargetDesc &targets = instanceDesc.targets;

        if (targets.isMainWindow()) {
            check(m_desc.colourAttachments.size() == 1);
            check(m_desc.colourAttachments[0].format == g_mainWindow->format());
            check(!m_desc.depthStencilAttachment);

            check(targets.colour[0].mip == 0);
            check(targets.colour[0].layer == 0);

            width = g_mainWindow->width();
            height = g_mainWindow->height();
        } else {
            /* Check if we have all expected colour attachments. */
            check(targets.colour.size() == m_desc.colourAttachments.size());
            if (m_desc.colourAttachments.size()) {
                for (size_t i = 0; i < targets.colour.size(); i++) {
                    check(targets.colour[i]);

                    /* Format must match. */
                    check(targets.colour[i].texture->format() == m_desc.colourAttachments[i].format);

                    /* All targets must be the same size. */
                    if (i == 0) {
                        width = targets.colour[i].texture->width();
                        height = targets.colour[i].texture->height();
                    } else {
                        check(targets.colour[i].texture->width() == width);
                        check(targets.colour[i].texture->height() == height);
                    }
                }
            }

            if (m_desc.depthStencilAttachment) {
                check(targets.depthStencil);
                check(targets.depthStencil.texture->format() == m_desc.depthStencilAttachment.format);

                if (!width && !height) {
                    width = targets.depthStencil.texture->width();
                    height = targets.depthStencil.texture->height();
                } else {
                    /* Depth/stencil size must match colour size. */
                    check(targets.depthStencil.texture->width() == width);
                    check(targets.depthStencil.texture->height() == height);
                }
            }
        }

        check(instanceDesc.clearColours.size() == m_desc.colourAttachments.size());

        check(instanceDesc.renderArea.x >= 0 && instanceDesc.renderArea.width >= 0);
        check(static_cast<uint32_t>(instanceDesc.renderArea.x + instanceDesc.renderArea.width) <= width);
        check(instanceDesc.renderArea.y >= 0 && instanceDesc.renderArea.height >= 0);
        check(static_cast<uint32_t>(instanceDesc.renderArea.y + instanceDesc.renderArea.height) <= height);
    #endif

    return new GPURenderPassInstance(instanceDesc);
}
