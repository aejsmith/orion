/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Vertex data class.
 */

#include "gpu/vertex_data.h"

/**
 * Initialize the vertex data object.
 *
 * Initialize the vertex data object. Initially the object has no buffers and
 * no vertex format associated with it. Users must set the buffers and format
 * to use before using it for rendering.
 *
 * @param count		Total number of vertices.
 */
VertexData::VertexData(size_t count) :
	m_count(count),
	m_format(nullptr),
	m_finalized(false)
{}

/**
 * Set the vertex format.
 *
 * Sets the vertex format for the data. The specified format descriptor must
 * be finalized.
 *
 * @param format	Vertex format descriptor.
 */
void VertexData::setFormat(const VertexFormatPtr &format) {
	orionAssert(!m_finalized);
	orionAssert(!m_format);
	orionAssert(format->finalized());

	m_format = format;
}

/**
 * Add a buffer.
 *
 * Adds a vertex buffer. Multiple vertex buffers can be attached, each is given
 * an index which is referenced by the vertex format. The specified index must
 * be specified as part of the vertex format. The buffer size must equal the
 * number of vertices multiplied by the stride specified by the vertex format.
 *
 * @param index		Index to add buffer at.
 * @param buffer	Buffer to add.
 */
void VertexData::setBuffer(unsigned index, const GPUBufferPtr &buffer) {
	orionAssert(!m_finalized);
	orionAssert(m_format);
	orionAssert(buffer->type() == GPUBuffer::kVertexBuffer);

	const VertexBufferDesc *desc = m_format->buffer(index);
	orionCheck(desc, "Format has no buffer at index %u", index);
	orionAssert(buffer->size() == (m_count * desc->stride));

	m_buffers.insert(m_buffers.begin() + index, buffer);
}

/** Finalize the vertex data object. */
void VertexData::finalize() {
	orionAssert(m_format);

	/* Each buffer specified by the format must be bound. */
	for(size_t i = 0; i < m_format->buffers().size(); i++) {
		if(m_format->buffer(i)) {
			orionCheck(
				i < m_buffers.size() && m_buffers[i],
				"Format requires buffer %u but no buffer bound", i);
		}
	}

	finalizeImpl();
	m_finalized = true;
}
