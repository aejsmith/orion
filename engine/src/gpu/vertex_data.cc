/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Vertex data class.
 */

#include "gpu/vertex_data.h"

/** Initialize the vertex data object.
 * @param count		Total number of vertices.
 * @param format	Vertex format.
 * @param buffers	Array of buffers, as required by the vertex format.
 *			Array is invalidated. */
GPUVertexData::GPUVertexData(size_t count, GPUVertexFormat *format, GPUBufferArray &buffers) :
	m_count(count),
	m_format(format),
	m_buffers(std::move(buffers))
{
	check(count);
	checkMsg(m_buffers.size() == format->buffers().size(),
		"Buffer count mismatch (expected %u, got %u)",
		format->buffers().size(), m_buffers.size());

	for(size_t i = 0; i < m_buffers.size(); i++) {
		check(m_buffers[i]);
		check(m_buffers[i]->type() == GPUBuffer::kVertexBuffer);
	}
}
