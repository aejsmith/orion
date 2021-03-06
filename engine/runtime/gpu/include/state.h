/*
 * Copyright (C) 2015-2017 Alex Smith
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

#include <vector>

/** Base GPU state object class.
 * @tparam Desc         Type of the state descriptor structure. */
template <typename Desc>
class GPUState : public GPUObject {
public:
    /** @return             Descriptor used to create the state object. */
    const Desc &desc() const { return m_desc; }
protected:
    explicit GPUState(const Desc &desc) : m_desc(desc) {}
    explicit GPUState(Desc &&desc) : m_desc(std::move(desc)) {}
    ~GPUState() {}

    Desc m_desc;                    /**< State descriptor. */

    /* For default creation methods in GPUManager. */
    friend class GPUManager;
};

/** Blending state descriptor. */
struct GPUBlendStateDesc {
    BlendFunc func;                 /**< Colour blending function. */
    BlendFactor sourceFactor;       /**< Source colour factor. */
    BlendFactor destFactor;         /**< Destination colour factor. */

    BlendFunc alphaFunc;            /**< Alpha blending function. */
    BlendFactor sourceAlphaFactor;  /**< Source alpha factor. */
    BlendFactor destAlphaFactor;    /**< Destination alpha factor. */

    GPUBlendStateDesc() :
        func              (BlendFunc::kAdd),
        sourceFactor      (BlendFactor::kOne),
        destFactor        (BlendFactor::kZero),
        alphaFunc         (BlendFunc::kAdd),
        sourceAlphaFactor (BlendFactor::kOne),
        destAlphaFactor   (BlendFactor::kZero)
    {}

    SET_DESC_PARAMETER(setFunc, BlendFunc, func);
    SET_DESC_PARAMETER(setSourceFactor, BlendFactor, sourceFactor);
    SET_DESC_PARAMETER(setDestFactor, BlendFactor, destFactor);

    SET_DESC_PARAMETER(setAlphaFunc, BlendFunc, alphaFunc);
    SET_DESC_PARAMETER(setSourceAlphaFactor, BlendFactor, sourceAlphaFactor);
    SET_DESC_PARAMETER(setDestAlphaFactor, BlendFactor, destAlphaFactor);

    /** Compare this descriptor with another. */
    bool operator ==(const GPUBlendStateDesc &other) const {
        return func == other.func &&
               sourceFactor == other.sourceFactor &&
               destFactor == other.destFactor &&
               alphaFunc == other.alphaFunc &&
               sourceAlphaFactor == other.sourceAlphaFactor &&
               destAlphaFactor == other.destAlphaFactor;
    }

    /** Get a hash from a blend state descriptor. */
    friend size_t hashValue(const GPUBlendStateDesc &desc) {
        size_t hash = hashValue(desc.func);
        hash = hashCombine(hash, desc.sourceFactor);
        hash = hashCombine(hash, desc.destFactor);
        hash = hashCombine(hash, desc.alphaFunc);
        hash = hashCombine(hash, desc.sourceAlphaFactor);
        hash = hashCombine(hash, desc.destAlphaFactor);
        return hash;
    }
};

/** Blend state object. */
using GPUBlendState = GPUState<GPUBlendStateDesc>;

/** Type of a pointer to a GPU blend state object. */
using GPUBlendStatePtr = GPUObjectPtr<GPUBlendState>;

/** Depth/stencil state descriptor. */
struct GPUDepthStencilStateDesc {
    ComparisonFunc depthFunc;       /**< Depth comparison function. */
    bool depthWrite;                /**< Whether to enable depth buffer writes. */

    GPUDepthStencilStateDesc() :
        depthFunc  (ComparisonFunc::kLessOrEqual),
        depthWrite (true)
    {}

    SET_DESC_PARAMETER(setDepthFunc, ComparisonFunc, depthFunc);
    SET_DESC_PARAMETER(setDepthWrite, bool, depthWrite);

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
using GPUDepthStencilState = GPUState<GPUDepthStencilStateDesc>;

/** Type of a pointer to a GPU depth/stencil state object. */
using GPUDepthStencilStatePtr = GPUObjectPtr<GPUDepthStencilState>;

/** Rasterizer state descriptor. */
struct GPURasterizerStateDesc {
    CullMode cullMode;              /**< Face culling mode. */
    bool depthClamp;                /**< Whether to enable depth clamping. */

    GPURasterizerStateDesc() :
        cullMode   (CullMode::kBack),
        depthClamp (false)
    {}

    SET_DESC_PARAMETER(setCullMode, CullMode, cullMode);
    SET_DESC_PARAMETER(setDepthClamp, bool, depthClamp);

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
using GPURasterizerState = GPUState<GPURasterizerStateDesc>;

/** Type of a pointer to a GPU rasterizer state object. */
using GPURasterizerStatePtr = GPUObjectPtr<GPURasterizerState>;

/** Texture sampler state descriptor. */
struct GPUSamplerStateDesc {
    SamplerFilterMode filterMode;   /**< Filtering mode. */
    unsigned maxAnisotropy;         /**< Anisotropic filtering level. */
    SamplerAddressMode addressU;    /**< Addressing mode in U direction. */
    SamplerAddressMode addressV;    /**< Addressing mode in V direction. */
    SamplerAddressMode addressW;    /**< Addressing mode in W direction. */
    bool compareEnable;             /**< Enable comparison against a reference value. */
    ComparisonFunc compareFunc;     /**< Comparison function when compareEnable is true. */

    GPUSamplerStateDesc() :
        filterMode    (SamplerFilterMode::kNearest),
        maxAnisotropy (1),
        addressU      (SamplerAddressMode::kClamp),
        addressV      (SamplerAddressMode::kClamp),
        addressW      (SamplerAddressMode::kClamp),
        compareEnable (false),
        compareFunc   (ComparisonFunc::kAlways)
    {}

    SET_DESC_PARAMETER(setFilterMode, SamplerFilterMode, filterMode);
    SET_DESC_PARAMETER(setMaxAnisotropy, unsigned, maxAnisotropy);
    SET_DESC_PARAMETER(setAddressU, SamplerAddressMode, addressU);
    SET_DESC_PARAMETER(setAddressV, SamplerAddressMode, addressV);
    SET_DESC_PARAMETER(setAddressW, SamplerAddressMode, addressW);
    SET_DESC_PARAMETER(setCompareEnable, bool, compareEnable);
    SET_DESC_PARAMETER(setCompareFunc, ComparisonFunc, compareFunc);

    /** Compare this descriptor with another. */
    bool operator ==(const GPUSamplerStateDesc &other) const {
        return filterMode == other.filterMode &&
               maxAnisotropy == other.maxAnisotropy &&
               addressU == other.addressU &&
               addressV == other.addressV &&
               addressW == other.addressW &&
               compareEnable == other.compareEnable &&
               compareFunc == other.compareFunc;
    }

    /** Get a hash from a sampler state descriptor. */
    friend size_t hashValue(const GPUSamplerStateDesc &desc) {
        size_t hash = hashValue(desc.filterMode);
        hash = hashCombine(hash, desc.maxAnisotropy);
        hash = hashCombine(hash, desc.addressU);
        hash = hashCombine(hash, desc.addressV);
        hash = hashCombine(hash, desc.addressW);
        hash = hashCombine(hash, desc.compareEnable);
        hash = hashCombine(hash, desc.compareFunc);
        return hash;
    }
};

/** Texture sampler state object. */
using GPUSamplerState = GPUState<GPUSamplerStateDesc>;

/** Type of a pointer to a GPU sampler state object. */
using GPUSamplerStatePtr = GPUObjectPtr<GPUSamplerState>;
