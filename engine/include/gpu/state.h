/*
 * Copyright (C) 2015-2016 Alex Smith
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
    GPUState(const Desc &desc) : m_desc(desc) {}
    GPUState(Desc &&desc) : m_desc(std::move(desc)) {}
    ~GPUState() {}

    Desc m_desc;                    /**< State descriptor. */

    /* For default creation methods in GPUManager. */
    friend class GPUManager;
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
using GPUBlendState = GPUState<GPUBlendStateDesc>;

/** Type of a pointer to a GPU blend state object. */
using GPUBlendStatePtr = GPUObjectPtr<GPUBlendState>;

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
using GPUDepthStencilState = GPUState<GPUDepthStencilStateDesc>;

/** Type of a pointer to a GPU depth/stencil state object. */
using GPUDepthStencilStatePtr = GPUObjectPtr<GPUDepthStencilState>;

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
using GPUSamplerState = GPUState<GPUSamplerStateDesc>;

/** Type of a pointer to a GPU sampler state object. */
using GPUSamplerStatePtr = GPUObjectPtr<GPUSamplerState>;

/**
 * Structure describing a vertex buffer binding.
 *
 * This structure describes layout information for a buffer to be used with a
 * vertex format. Currently it only defines the stride between each vertex,
 * everything else is described by the attributes.
 */
struct VertexBinding {
    size_t stride;                  /**< Offset between each vertex. */
};

/**
 * Structure describing a vertex attribute.
 *
 * This structure describes a single vertex attribute. An attribute can be
 * bound to a variable in a shader and then used to retrieve vertex data. An
 * attribute has a semantic and an index that is used to bind shader variables.
 * The index allows multiple attributes with the same semantic (for example,
 * multiple sets of texture coordinates).
 */
struct VertexAttribute {
    /** List of attribute semantics. */
    enum Semantic {
        kPositionSemantic,          /**< Vertex position. */
        kNormalSemantic,            /**< Vertex normal. */
        kTexcoordSemantic,          /**< Texture coordinates. */
        kDiffuseSemantic,           /**< Diffuse colour. */
        kSpecularSemantic,          /**< Specular colour. */
    };

    /** Enumeration of attribute data types. */
    enum Type {
        kByteType,                  /**< Signed 8-bit integer. */
        kUnsignedByteType,          /**< Unsigned 8-bit integer. */
        kShortType,                 /**< Signed 16-bit integer. */
        kUnsignedShortType,         /**< Unsigned 16-bit integer. */
        kIntType,                   /**< Signed 32-bit integer. */
        kUnsignedIntType,           /**< Unsigned 32-bit integer. */
        kFloatType,                 /**< Single-precision floating point. */
        kDoubleType,                /**< Double-precision floating point. */
    };

    Semantic semantic;              /**< Semantic of the attribute. */
    unsigned index;                 /**< Attribute index. */
    Type type;                      /**< Attribute data type. */
    bool normalised;                /**< Whether fixed-point values should be normalised when accessed. */
    size_t components;              /**< Number of components (for vector types). */
    unsigned binding;               /**< Index of binding that will contain the attribute. */
    size_t offset;                  /**< Offset of the attribute within each vertex in the buffer. */

    /** @return             Size of the attribute in bytes. */
    size_t size() const { return size(this->type, this->components); }

    /** Get the size of a vertex attribute type.
     * @param type          Type to get size of.
     * @param count         Number of elements (for vector types). */
    static size_t size(Type type, size_t components = 1) {
        switch (type) {
            case kByteType:
            case kUnsignedByteType:
                return sizeof(uint8_t) * components;
            case kShortType:
            case kUnsignedShortType:
                return sizeof(uint16_t) * components;
            case kIntType:
            case kUnsignedIntType:
                return sizeof(uint32_t) * components;
            case kFloatType:
                return sizeof(float) * components;
            case kDoubleType:
                return sizeof(double) * components;
            default:
                return 0;
        }
    }
};

/** Vertex input state descriptor. */
struct GPUVertexInputStateDesc {
    /** Vertex buffer binding descriptions. */
    std::vector<VertexBinding> bindings;

    /** Vertex attribute descriptions. */
    std::vector<VertexAttribute> attributes;
public:
    /** Initialise an empty vertex input state. */
    GPUVertexInputStateDesc() {}

    /** Initialise with pre-allocated arrays.
     * @param numBindings   Number of bindings.
     * @param numAttributes Number of attributes. */
    GPUVertexInputStateDesc(size_t numBindings, size_t numAttributes) :
        bindings(numBindings),
        attributes(numAttributes)
    {}
};

/**
 * Vertex input state information.
 *
 * This class holds a description of the layout of vertex data across one or
 * more GPU buffers. This information includes the offset between each vertex
 * in the buffer (the stride), and the vertex attributes contained across the
 * buffers.
 */
class GPUVertexInputState : public GPUState<GPUVertexInputStateDesc> {
protected:
    GPUVertexInputState(GPUVertexInputStateDesc &&desc);

    /* For default creation method in GPUManager. */
    friend class GPUManager;
};

/** Type of a pointer to a GPU vertex input state object. */
using GPUVertexInputStatePtr = GPUObjectPtr<GPUVertexInputState>;
