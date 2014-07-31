/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Vertex data class.
 */

#include "core/engine.h"

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

/** Destroy the vertex data object. */
VertexData::~VertexData() {}

/**
 * Set the vertex format.
 *
 * Sets the vertex format for the data. The specified format descriptor must
 * be finalized.
 *
 * @param format	Vertex format descriptor.
 */
void VertexData::set_format(const VertexFormatPtr &format) {
	orion_assert(!m_finalized);
	orion_assert(!m_format);
	orion_assert(format->finalized());

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
void VertexData::set_buffer(unsigned index, const GPUBufferPtr &buffer) {
	orion_assert(!m_finalized);
	orion_assert(m_format);
	orion_assert(buffer->type() == GPUBuffer::kVertexBuffer);

	const VertexBufferDesc *desc = m_format->buffer(index);
	orion_check(desc, "Format has no buffer at index %u", index);
	orion_assert(buffer->size() == (m_count * desc->stride));

	m_buffers.insert(m_buffers.begin() + index, buffer);
}

/** Finalize the vertex data object. */
void VertexData::finalize() {
	orion_assert(m_format);

	/* Each buffer specified by the format must be bound. */
	for(size_t i = 0; i < m_format->buffers().size(); i++) {
		if(m_format->buffer(i)) {
			orion_check(
				i < m_buffers.size() && m_buffers[i],
				"Format requires buffer %u but no buffer bound", i);
		}
	}

	_finalize();
	m_finalized = true;
}
