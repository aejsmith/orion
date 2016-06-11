/*
 * Copyright (C) 2015 Alex Smith
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
 * @brief               GPU state objects.
 */

#pragma once

#include "core/hash.h"

#include "gpu/defs.h"

/** Base GPU state object class.
 * @tparam Desc         Type of the state descriptor structure. */
template <typename Desc>
class GPUState : public GPUResource {
public:
    /** @return             Descriptor used to create the state object. */
    const Desc &desc() const { return m_desc; }
protected:
    GPUState(const Desc &desc) : m_desc(desc) {}
    ~GPUState() {}

    Desc m_desc;                    /**< State descriptor. */
};

/** Blending state descriptor. */
struct GPUBlendStateDesc {
    BlendFunc func;                 /**< Blending function. */
    BlendFactor sourceFactor;       /**< Source factor. */
    BlendFactor destFactor;         /**< Destination factor. */
public:
    /** Compare this descriptor with another. */
    bool operator ==(const GPUBlendStateDesc &other) const {
        return func == other.func && sourceFactor == other.sourceFactor && destFactor == other.destFactor;
    }

    /** Get a hash from a blend state descriptor. */
    friend size_t hashValue(const GPUBlendStateDesc &desc) {
        size_t hash = hashValue(desc.func);
        hash = hashCombine(hash, desc.sourceFactor);
        hash = hashCombine(hash, desc.destFactor);
        return hash;
    }
};

/** Blend state object. */
typedef GPUState<GPUBlendStateDesc> GPUBlendState;

/** Type of a pointer to a GPU blend state object. */
typedef GPUResourcePtr<GPUBlendState> GPUBlendStatePtr;

/** Depth/stencil state descriptor. */
struct GPUDepthStencilStateDesc {
    ComparisonFunc depthFunc;       /**< Depth comparison function. */
    bool depthWrite;                /**< Whether to enable depth buffer writes. */
public:
    /** Compare this descriptor with another. */
    bool operator ==(const GPUDepthStencilStateDesc &other) const {
        return depthFunc == other.depthFunc && depthWrite == other.depthWrite;
    }

    /** Get a hash from a depth/stencil state descriptor. */
    friend size_t hashValue(const GPUDepthStencilStateDesc &desc) {
        size_t hash = hashValue(desc.depthFunc);
        hash = hashCombine(hash, desc.depthWrite);
        return hash;
    }
};

/** Depth/stencil state object. */
typedef GPUState<GPUDepthStencilStateDesc> GPUDepthStencilState;

/** Type of a pointer to a GPU depth/stencil state object. */
typedef GPUResourcePtr<GPUDepthStencilState> GPUDepthStencilStatePtr;

/** Rasterizer state descriptor. */
struct GPURasterizerStateDesc {
    CullMode cullMode;              /**< Face culling mode. */
    bool depthClamp;                /**< Whether to enable depth clamping. */
public:
    /** Compare this descriptor with another. */
    bool operator ==(const GPURasterizerStateDesc &other) const {
        return cullMode == other.cullMode && depthClamp == other.depthClamp;
    }

    /** Get a hash from a rasterizer state descriptor. */
    friend size_t hashValue(const GPURasterizerStateDesc &desc) {
        size_t hash = hashValue(desc.cullMode);
        hash = hashCombine(hash, desc.depthClamp);
        return hash;
    }
};

/** Rasterizer state object. */
typedef GPUState<GPURasterizerStateDesc> GPURasterizerState;

/** Type of a pointer to a GPU rasterizer state object. */
typedef GPUResourcePtr<GPURasterizerState> GPURasterizerStatePtr;

/** Texture sampler state descriptor. */
struct GPUSamplerStateDesc {
    SamplerFilterMode filterMode;   /**< Filtering mode. */
    unsigned maxAnisotropy;         /**< Anisotropic filtering level. */
    SamplerAddressMode addressU;    /**< Addressing mode in U direction. */
    SamplerAddressMode addressV;    /**< Addressing mode in V direction. */
    SamplerAddressMode addressW;    /**< Addressing mode in W direction. */
public:
    /** Compare this descriptor with another. */
    bool operator ==(const GPUSamplerStateDesc &other) const {
        return filterMode == other.filterMode &&
            maxAnisotropy == other.maxAnisotropy &&
            addressU == other.addressU &&
            addressV == other.addressV &&
            addressW == other.addressW;
    }

    /** Get a hash from a sampler state descriptor. */
    friend size_t hashValue(const GPUSamplerStateDesc &desc) {
        size_t hash = hashValue(desc.filterMode);
        hash = hashCombine(hash, desc.maxAnisotropy);
        hash = hashCombine(hash, desc.addressU);
        hash = hashCombine(hash, desc.addressV);
        hash = hashCombine(hash, desc.addressW);
        return hash;
    }
};

/** Texture sampler state object. */
typedef GPUState<GPUSamplerStateDesc> GPUSamplerState;

/** Type of a pointer to a GPU sampler state object. */
typedef GPUResourcePtr<GPUSamplerState> GPUSamplerStatePtr;
