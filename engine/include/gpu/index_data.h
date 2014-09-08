/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Index data class.
 */

#pragma once

#include "gpu/buffer.h"

/**
 * Class which collects index data.
 *
 * This class holds a buffer containing index data and information about the
 * format of that buffer. It is used to provide indices into vertex data to be
 * used for rendering. Since this class may have an API-specific implementation,
 * instances must be created with GPUInterface::create_index_data().
 */
class IndexData : public GPUResource {
public:
	/** Type of index elements. */
	enum Type {
		kUnsignedByteType,	/**< Unsigned 8-bit. */
		kUnsignedShortType,	/**< Unsigned 16-bit. */
		kUnsignedIntType,	/**< Unsigned 32-bit. */
	};
public:
	/** @return		Buffer containing index data. */
	GPUBufferPtr buffer() const { return m_buffer; }
	/** @return		Type of index elements. */
	Type type() const { return m_type; }
	/** @return		Number of indices. */
	size_t count() const { return m_count; }
	/** @return		Size of a single index element. */
	size_t element_size() const { return element_size(m_type); }

	/** Get the size of a buffer element of a certain type.
	 * @param type		Type of buffer element.
	 * @return		Size of element. */
	static size_t element_size(Type type) {
		switch(type) {
		case kUnsignedByteType:
			return 1;
		case kUnsignedShortType:
			return 2;
		case kUnsignedIntType:
			return 4;
		default:
			return 0;
		}
	}
protected:
	IndexData(const GPUBufferPtr &buffer, Type type, size_t count);
protected:
	GPUBufferPtr m_buffer;		/**< Buffer containing index data. */
	Type m_type;			/**< Type of index elements. */
	size_t m_count;			/**< Number of indices. */

	/* For the default implementation of create_index_data(). */
	friend class GPUInterface;
};

/** Shared pointer to IndexData. */
typedef GPUResourcePtr<IndexData> IndexDataPtr;
