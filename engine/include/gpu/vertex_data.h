/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Vertex data class.
 */

#pragma once

#include "gpu/buffer.h"
#include "gpu/vertex_format.h"

#include <vector>

/**
 * Class which collects vertex data information.
 *
 * This class collects one or more vertex buffers and a vertex format describing
 * the vertex attributes which are contained in the buffers.
 *
 * Usage of this class should be as follows:
 *
 *  1. Set vertex format with setFormat().
 *  2. Set buffers with setBuffer().
 *  3. Finalize the object with finalize().
 *
 * The final step allows the API-specific implementation to compile it's own
 * objects from the provided buffer (for example, the GL backend produces a
 * VAO).
 *
 * Since this class may have an API-specific implementation, instances must be
 * created with GPUInterface::createVertexData().
 */
class VertexData : public GPUResource {
public:
	void setFormat(VertexFormat *format);
	void setBuffer(unsigned index, GPUBuffer *buffer);

	void finalize();

	/** @return		Total number of vertices. */
	size_t count() const { return m_count; }
	/** @return		Pointer to vertex format descriptor. */
	VertexFormat *format() const { return m_format; }

	/** Get a vertex buffer.
	 * @param index		Buffer index to get.
	 * @return		Pointer to vertex buffer, or null if no buffer
	 *			at specified index. */
	GPUBuffer *buffer(unsigned index) const {
		return (index < m_buffers.size()) ? m_buffers[index] : nullptr;
	}

	/** @return		Whether the format is finalized. */
	bool finalized() const { return m_finalized; }
protected:
	/** Type of a GPU buffer array. */
	typedef std::vector<GPUBufferPtr> GPUBufferArray;
protected:
	explicit VertexData(size_t count);

	/** Called when the object is being finalized. */
	virtual void finalizeImpl() {}
protected:
	size_t m_count;			/**< Vertex count. */
	VertexFormatPtr m_format;	/**< Vertex format. */
	GPUBufferArray m_buffers;	/**< Vector of vertex buffers. */
	bool m_finalized;		/**< Whether the object has been finalized. */

	/* For the default implementation of createVertexData(). */
	friend class GPUInterface;
};

/** Shared pointer to VertexData. */
typedef GPUResourcePtr<VertexData> VertexDataPtr;
