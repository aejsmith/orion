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
 *
 * TODO:
 *  - For now we don't expose Vulkan subpass functionality through this, as
 *    using that requires special handling in shaders and means extra work to
 *    support this on GL (as we would have to transform shaders using this).
 */

#pragma once

#include "core/pixel_format.h"

#include "gpu/state.h"
#include "gpu/texture.h"

struct GPURenderPassInstanceDesc;

/** Possible ways to treat existing attachment contents at start of pass. */
enum class GPURenderLoadOp {
    kLoad,                              /**< Preserve existing contents. */
    kClear,                             /**< Clear to the value specified when starting the render pass. */
    kDontCare,                          /**< Don't care about the existing value, will be undefined. */
};

/** Structure describing a render pass attachment. */
struct GPURenderAttachmentDesc {
    PixelFormat format;                 /**< Pixel format of the attachment. */
    GPURenderLoadOp loadOp;             /**< How to treat existing colour/depth target contents at start of pass. */
    GPURenderLoadOp stencilLoadOp;      /**< How to treat existing stencil target contents at start of pass. */

    /** Initialise as an unused attachment. */
    GPURenderAttachmentDesc() :
        format(PixelFormat::kUnknown),
        loadOp(GPURenderLoadOp::kDontCare),
        stencilLoadOp(GPURenderLoadOp::kDontCare)
    {}

    /** @return             Whether this is a used attachment. */
    explicit operator bool() const {
        return format != PixelFormat::kUnknown;
    }
};

/** Structure describing a render pass. */
struct GPURenderPassDesc {
    /** Array of colour attachment descriptors. */
    std::vector<GPURenderAttachmentDesc> colourAttachments;

    /** Depth/stencil attachment. */
    GPURenderAttachmentDesc depthStencilAttachment;

    /** Initialise the descriptor.
     * @param numColour     Number of colour attachments. */
    explicit GPURenderPassDesc(size_t numColour = 0) :
        colourAttachments(numColour)
    {}
};

/**
 * Render pass.
 *
 * This object describes a render pass. Render pass objects are created up front
 * and provide information about the attachments which will be rendered to by
 * the pass. This information includes the pixel format of the attachment and
 * how to treat the existing contents of it. The actual render targets which
 * will be used are not specified. These are specified when beginning a render
 * pass instance, and must be compatible with the render pass object.
 */
class GPURenderPass : public GPUState<GPURenderPassDesc> {
public:
    void validateInstance(const GPURenderPassInstanceDesc &instanceDesc) const;
protected:
    explicit GPURenderPass(GPURenderPassDesc &&desc);

    /* For default creation method in GPUManager. */
    friend class GPUManager;
};

#ifndef ORION_BUILD_DEBUG

inline void GPURenderPass::validateInstance(const GPURenderPassInstanceDesc &instanceDesc) const {}

#endif

/** Type of a reference to GPURenderPass. */
using GPURenderPassPtr = GPUObjectPtr<GPURenderPass>;

/**
 * Render target descriptor structure.
 *
 * This structure describes the textures to be rendered to in a render pass
 * instance. The render target layout and format of each image used must be
 * compatible with the render pass' attachment description.
 *
 * As a special case, if a render target description has 1 colour target which
 * is a null image reference, and a null depth/stencil target, then it refers
 * to the main window.
 */
struct GPURenderTargetDesc {
    /** Array of colour render target descriptors. */
    std::vector<GPUTextureImageRef> colour;

    /** Depth/stencil target. */
    GPUTextureImageRef depthStencil;

    /** Initialise the descriptor.
     * @param numColour     Number of colour targets. */
    explicit GPURenderTargetDesc(size_t numColour = 0) :
        colour(numColour)
    {}

    /** Compare this descriptor with another. */
    bool operator ==(const GPURenderTargetDesc &other) const {
        if (colour.size() != other.colour.size() || depthStencil != other.depthStencil)
            return false;

        for (size_t i = 0; i < colour.size(); i++) {
            if (colour[i] != other.colour[i])
                return false;
        }

        return true;
    }

    /** @return             Whether this descriptor refers to the main window. */
    bool isMainWindow() const {
        return colour.size() == 1 && !colour[0] && !depthStencil;
    }

    /** Get a hash from a render target descriptor. */
    friend size_t hashValue(const GPURenderTargetDesc &desc) {
        size_t hash = hashValue(desc.colour.size());

        for (size_t i = 0; i < desc.colour.size(); i++)
            hash = hashCombine(hash, desc.colour[i]);

        hash = hashCombine(hash, desc.depthStencil);

        return hash;
    }
};

/**
 * Render pass instance description.
 *
 * This specifies an instance of a render pass. This includes the actual render
 * targets that will be used, clear values for any attachments with their load
 * operation set to clear, and an area which will be affected by the pass.
 */
struct GPURenderPassInstanceDesc {
    const GPURenderPass *pass;          /**< Render pass. */

    /** Render targets to use. Must be compatible with the pass. */
    GPURenderTargetDesc targets;

    /** Clear values for colour targets. */
    std::vector<glm::vec4> clearColours;

    float clearDepth;                   /**< Clear value for the depth target. */
    uint32_t clearStencil;              /**< Clear value for the stencil target. */

    /**
     * Area which will be affected by the render pass.
     *
     * This limits the area that will be affected by the render pass. Load
     * operations only apply to the constrained area, and rendering will not
     * affect any pixels outside the area. This area is also the upper limit
     * on viewport size throughout the pass, and the viewport will be initially
     * set to this at the beginning of the pass.
     */
    IntRect renderArea;

    /** Initialise the descriptor (pre-allocates vectors with correct size).
     * @param inPass        Pass the instance is for. */
    explicit GPURenderPassInstanceDesc(const GPURenderPass *inPass) :
        pass(inPass),
        targets(inPass->desc().colourAttachments.size()),
        clearColours(inPass->desc().colourAttachments.size())
    {}
};
