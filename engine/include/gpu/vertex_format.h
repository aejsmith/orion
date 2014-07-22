/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Vertex format class.
 */

#ifndef ORION_GPU_VERTEX_FORMAT_H
#define ORION_GPU_VERTEX_FORMAT_H

#include "core/defs.h"

#include <list>
#include <memory>
#include <vector>

/**
 * Structure describing a vertex buffer.
 *
 * This structure describes additional layout information for a buffer to be
 * used with a vertex format. Currently it only defines the stride between
 * each vertex, everything else is described by the attributes.
 */
struct VertexBufferDesc {
	size_t stride;			/**< Offset between each vertex. */
public:
	VertexBufferDesc() : stride(0) {}
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
		kPositionSemantic,	/**< Vertex position. */
		kNormalSemantic,	/**< Vertex normal. */
		kTexCoordSemantic,	/**< Texture coordinates. */
		kDiffuseSemantic,	/**< Diffuse colour. */
		kSpecularSemantic,	/**< Specular colour. */
	};

	/** Enumeration of attribute data types. */
	enum Type {
		kByteType,		/**< Signed 8-bit integer. */
		kUnsignedByteType,	/**< Unsigned 8-bit integer. */
		kShortType,		/**< Signed 16-bit integer. */
		kUnsignedShortType,	/**< Unsigned 16-bit integer. */
		kIntType,		/**< Signed 32-bit integer. */
		kUnsignedIntType,	/**< Unsigned 32-bit integer. */
		kFloatType,		/**< Single-precision floating point. */
		kDoubleType,		/**< Double-precision floating point. */
	};

	Semantic semantic;		/**< Semantic of the attribute. */
	unsigned index;			/**< Attribute index. */
	Type type;			/**< Attribute data types. */
	size_t count;			/**< Number of elements (for vector types). */
	unsigned buffer;		/**< Buffer containing the attribute. */
	size_t offset;			/**< Offset of the attribute within each vertex in the buffer. */
public:
	static size_t size(Type type, size_t count = 1);

	/** Get the size of the attribute.
	 * @return		Size of the attribute. */
	size_t size() const { return size(type, count); }
};

/** Get the size of a vertex attribute type.
 * @param type		Type to get size of.
 * @param count		Number of elements (for vector types). */
inline size_t VertexAttribute::size(Type type, size_t count) {
	switch(type) {
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

/**
 * Vertex format descriptor.
 *
 * This class holds a description of the layout of vertex data across one or
 * more GPU buffers. This information includes the offset between each vertex
 * in the buffer (the stride), and the vertex attributes contained across the
 * buffers.
 *
 * Usage of this class should be as follows:
 *
 *  1. Add buffer descriptions with add_buffer().
 *  2. Add attribute descriptions with add_attribute().
 *  3. Finalize the object with finalize().
 *
 * The last step allows the API-specific implementation to compile any
 * information it needs, and makes the object immutable. A vertex format must
 * be finalized before it is assigned to a VertexData object.
 *
 * Since this class may have an API-specific implementation, instances must be
 * created with GPUInterface::create_vertex_format().
 */
class VertexFormat {
public:
	/** Type of the buffer array. */
	typedef std::vector<VertexBufferDesc> BufferArray;

	/** Type of the attribute list. */
	typedef std::list<VertexAttribute> AttributeList;
public:
	virtual ~VertexFormat();

	void add_buffer(unsigned index, size_t stride);
	void add_attribute(VertexAttribute::Semantic semantic, unsigned index,
		VertexAttribute::Type type, size_t count, unsigned buffer,
		size_t offset);

	virtual void finalize();

	const VertexBufferDesc *buffer(unsigned index) const;
	const VertexAttribute *find_attribute(VertexAttribute::Semantic semantic, unsigned index) const;

	/** Get the buffer array.
	 * @return		Buffer array. */
	const BufferArray &buffers() const { return m_buffers; }

	/** Get the attribute list.
	 * @return		Attribute list. */
	const AttributeList &attributes() const { return m_attributes; }

	/** Return whether the format is finalized.
	 * @return		Whether the format is finalized. */
	bool finalized() const { return m_finalized; }
protected:
	VertexFormat();

	/** Called when the object is being finalized. */
	virtual void _finalize() {}
protected:
	BufferArray m_buffers;		/**< Array of all buffer descriptors. */
	AttributeList m_attributes;	/**< List of all attributes. */
	bool m_finalized;		/**< Whether the descriptor is finalized. */

	/* For the default implementation of create_vertex_format(). */
	friend class GPUInterface;
};

/** Shared pointer to VertexFormat. */
typedef std::shared_ptr<VertexFormat> VertexFormatPtr;

#endif /* ORION_GPU_VERTEX_FORMAT_H */
