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
 * @brief               Vertex data class.
 */

#pragma once

#include "gpu/buffer.h"
#include "gpu/state.h"

#include <vector>

/**
 * Structure describing a vertex buffer binding.
 *
 * This structure describes layout information for a buffer to be used with a
 * vertex format. Currently it only defines the stride between each vertex,
 * everything else is described by the attributes.
 */
struct VertexBinding {
    size_t stride;                  /**< Offset between each vertex. */

    /** Compare this descriptor with another. */
    bool operator ==(const VertexBinding &other) const {
        return stride == other.stride;
    }

    /** Get a hash from a vertex binding descriptor. */
    friend size_t hashValue(const VertexBinding &desc) {
        return hashValue(desc.stride);
    }
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
        kNumTypes,
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

    /** @return             GLSL attribute index for this attribute. */
    unsigned glslIndex() const { return glslIndex(this->semantic, this->index); }

    static unsigned glslIndex(Semantic semantic, unsigned index);

    /** Compare this descriptor with another. */
    bool operator ==(const VertexAttribute &other) const {
        return
            semantic == other.semantic &&
            index == other.index &&
            type == other.type &&
            normalised == other.normalised &&
            components == other.components &&
            binding == other.binding &&
            offset == other.offset;
    }

    /** Get a hash from a vertex attribute descriptor. */
    friend size_t hashValue(const VertexAttribute &desc) {
        size_t hash = hashValue(desc.semantic);
        hash = hashCombine(hash, desc.index);
        hash = hashCombine(hash, desc.type);
        hash = hashCombine(hash, desc.normalised);
        hash = hashCombine(hash, desc.components);
        hash = hashCombine(hash, desc.binding);
        hash = hashCombine(hash, desc.offset);
        return hash;
    }
};

/** Vertex data layout descriptor. */
struct GPUVertexDataLayoutDesc {
    /** Vertex buffer binding descriptions. */
    std::vector<VertexBinding> bindings;

    /** Vertex attribute descriptions. */
    std::vector<VertexAttribute> attributes;

    /** Initialise with pre-allocated arrays.
     * @param numBindings   Number of bindings.
     * @param numAttributes Number of attributes. */
    explicit GPUVertexDataLayoutDesc(size_t numBindings = 0, size_t numAttributes = 0) :
        bindings(numBindings),
        attributes(numAttributes)
    {}

    /** Compare this descriptor with another. */
    bool operator ==(const GPUVertexDataLayoutDesc &other) const {
        return
            bindings == other.bindings &&
            attributes == other.attributes;
    }

    /** Get a hash from a vertex data layout descriptor. */
    friend size_t hashValue(const GPUVertexDataLayoutDesc &desc) {
        size_t hash = hashValue(desc.bindings.size());

        for (size_t i = 0; i < desc.bindings.size(); i++)
            hash = hashCombine(hash, desc.bindings[i]);

        hash = hashCombine(hash, desc.attributes.size());

        for (size_t i = 0; i < desc.attributes.size(); i++)
            hash = hashCombine(hash, desc.attributes[i]);

        return hash;
    }
};

/**
 * Vertex data layout information.
 *
 * This class holds a description of the layout of vertex data across one or
 * more GPU buffers. This information includes the offset between each vertex
 * in the buffer (the stride), and the vertex attributes contained across the
 * buffers.
 */
class GPUVertexDataLayout : public GPUState<GPUVertexDataLayoutDesc> {
protected:
    explicit GPUVertexDataLayout(const GPUVertexDataLayoutDesc &desc);

    /* For default creation method in GPUManager. */
    friend class GPUManager;
};

/** Type of a pointer to a GPU vertex data layout object. */
using GPUVertexDataLayoutPtr = GPUObjectPtr<GPUVertexDataLayout>;

/** Descriptor for a vertex data object. */
struct GPUVertexDataDesc {
    size_t count;                           /**< Vertex count. */
    GPUVertexDataLayoutPtr layout;          /**< Vertex data layout. */
    GPUBufferArray buffers;                 /**< Vector of vertex buffers. */

    GPUVertexDataDesc &setCount(size_t count) { this->count = count; return *this; }
    GPUVertexDataDesc &setLayout(GPUVertexDataLayout *layout) {
        this->layout = layout;
        this->buffers.resize(layout->desc().bindings.size());
        return *this;
    }
};

/**
 * Class which collects vertex data information.
 *
 * This class collects one or more vertex buffers and a layout object describing
 * the vertex attributes which are contained in the buffers. Once created, a
 * vertex data object is immutable. The vertex buffer contents can be changed,
 * but to change the vertex count or the buffers in use, a new vertex data
 * object must be created. Creation is performed through
 * GPUManager::createVertexData().
 */
class GPUVertexData : public GPUObject {
public:
    /** @return             Total number of vertices. */
    size_t count() const { return m_count; }
    /** @return             Pointer to vertex data layout. */
    GPUVertexDataLayout *layout() const { return m_layout; }
    /** @return             GPU buffer array. */
    const GPUBufferArray &buffers() const { return m_buffers; }
protected:
    explicit GPUVertexData(GPUVertexDataDesc &&desc);
    ~GPUVertexData() {}

    size_t m_count;                         /**< Vertex count. */
    GPUVertexDataLayoutPtr m_layout;        /**< Vertex data layout. */
    GPUBufferArray m_buffers;               /**< Vector of vertex buffers. */

    /* For the default implementation of createVertexData(). */
    friend class GPUManager;
};

/** Type of a reference to GPUVertexData. */
using GPUVertexDataPtr = GPUObjectPtr<GPUVertexData>;
