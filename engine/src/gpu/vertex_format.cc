/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Vertex format class.
 */

#include "gpu/vertex_format.h"

/** Initialize a vertex format descriptor.
 * @param buffers	Array of buffer layout descriptors. Array is invalidated.
 * @param attributes	Array of attribute descriptors. Array is invalidated. */
GPUVertexFormat::GPUVertexFormat(VertexBufferLayoutArray &buffers, VertexAttributeArray &attributes) :
	m_buffers(std::move(buffers)),
	m_attributes(std::move(attributes))
{
	for(size_t i = 0; i < m_buffers.size(); i++)
		check(m_buffers[i].stride);

	for(size_t i = 0; i < m_attributes.size(); i++) {
		const VertexAttribute &attribute = m_attributes[i];

		check(attribute.count);
		checkMsg(attribute.count >= 1 && attribute.count <= 4,
			"Attribute %u vector size %u unsupported", i, attribute.count);
		checkMsg(attribute.buffer < m_buffers.size(),
			"Attribute %u references unknown buffer %u", i, attribute.buffer);
		checkMsg((attribute.offset + attribute.size()) <= m_buffers[attribute.buffer].stride,
			"Attribute %u position exceeds buffer stride (offset: %u, size: %u, stride: %u)",
			i, attribute.offset, attribute.size(), m_buffers[attribute.buffer].stride);

		for(size_t j = 0; j < i; j++) {
			const VertexAttribute &other = m_attributes[j];

			checkMsg(other.semantic != attribute.semantic || other.index != attribute.index,
				"Attribute %u is semantic duplicate of %u", i, j);
		}
	}
}
