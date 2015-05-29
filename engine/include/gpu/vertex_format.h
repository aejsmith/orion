/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Vertex format class.
 */

#pragma once

#include "gpu/defs.h"

#include <vector>

/** Maximum number of vertex attributes. */
static const size_t kMaxVertexAttributes = 16;

/**
 * Structure describing a vertex buffer layout.
 *
 * This structure describes layout information for a buffer to be used with a
 * vertex format. Currently it only defines the stride between each vertex,
 * everything else is described by the attributes.
 */
struct VertexBufferLayout {
    size_t stride;                  /**< Offset between each vertex. */
public:
    VertexBufferLayout() : stride(0) {}
};

/** Type of a vertex buffer layout array. */
typedef std::vector<VertexBufferLayout> VertexBufferLayoutArray;

/**
 * Structure describing a vertex attribute.
 *
 * This structure describes a single vertex attribute. An attribute can be
 * bound to a variable in a shader and then used to retrieve vertex data. An
 * attribute has a semantic and an index that is used to bind shader variables.
 * The index allows multiple attributes with the same semantic (for example,
 * multiple sets of texture coordinates).
 *
 * In the array passed to GPUManager::createVertexData(), entries with a zero
 * count are ignored.
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
public:
    Semantic semantic;              /**< Semantic of the attribute. */
    unsigned index;                 /**< Attribute index. */
    Type type;                      /**< Attribute data types. */
    size_t count;                   /**< Number of elements (for vector types). */
    unsigned buffer;                /**< Index of buffer that will contain the attribute. */
    size_t offset;                  /**< Offset of the attribute within each vertex in the buffer. */
public:
    VertexAttribute() : count(0) {}

    /** @return             Size of the attribute in bytes. */
    size_t size() const { return size(type, count); }

    /** Get the size of a vertex attribute type.
     * @param type          Type to get size of.
     * @param count         Number of elements (for vector types). */
    static size_t size(Type type, size_t count = 1) {
        switch (type) {
            case kByteType:
            case kUnsignedByteType:
                return sizeof(uint8_t) * count;
            case kShortType:
            case kUnsignedShortType:
                return sizeof(uint16_t) * count;
            case kIntType:
            case kUnsignedIntType:
                return sizeof(uint32_t) * count;
            case kFloatType:
                return sizeof(float) * count;
            case kDoubleType:
                return sizeof(double) * count;
            default:
                return 0;
        }
    }
};

/** Type of a vertex attribute array. */
typedef std::vector<VertexAttribute> VertexAttributeArray;

/**
 * Vertex format information.
 *
 * This class holds a description of the layout of vertex data across one or
 * more GPU buffers. This information includes the offset between each vertex
 * in the buffer (the stride), and the vertex attributes contained across the
 * buffers.
 *
 * Once created, a vertex format is immutable. Creation is performed through
 * GPUManager::createVertexFormat(), which should be supplied with arrays
 * describing the buffer layouts and the attributes.
 */
class GPUVertexFormat : public GPUResource {
public:
    /** @return             Array of buffer descriptors. */
    const VertexBufferLayoutArray &buffers() const { return m_buffers; }
    /** @return             List of all attributes. */
    const VertexAttributeArray &attributes() const { return m_attributes; }
protected:
    GPUVertexFormat(VertexBufferLayoutArray &buffers, VertexAttributeArray &attributes);
protected:
    /** Array of buffer descriptors. */
    VertexBufferLayoutArray m_buffers;
    /** Array of attributes. */
    VertexAttributeArray m_attributes;

    /* For the default implementation of createVertexFormat(). */
    friend class GPUManager;
};

/** Type of a reference to a GPUVertexFormat. */
typedef GPUResourcePtr<GPUVertexFormat> GPUVertexFormatPtr;
