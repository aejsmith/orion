/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Vertex format class.
 */

#include "core/engine.h"

#include "gpu/vertex_format.h"

/** Initialize a vertex format descriptor. */
VertexFormat::VertexFormat() : m_finalized(false) {}

/** Add a buffer.
 * @see			VertexBufferDesc.
 * @param index		Index of the buffer to add. */
void VertexFormat::add_buffer(unsigned index, size_t stride) {
	orion_assert(stride);
	orion_assert(!m_finalized);
	orion_check(!buffer(index), "Adding duplicate buffer %u", index);

	if(index >= m_buffers.size())
		m_buffers.resize(index + 1);

	m_buffers[index].stride = stride;
}

/**
 * Add an attribute.
 *
 * Adds a vertex attribute to the format descriptor. The attribute semantic and
 * index are used to bind vertex data elements to shader variables. The index
 * allows multiple attributes with the same semantic (e.g. multiple sets of
 * texture coordinates). You cannot add multiple attributes with the same
 * semantic and index.
 *
 * @see			VertexAttribute.
 */
void VertexFormat::add_attribute(
	VertexAttribute::Semantic semantic,
	unsigned index,
	VertexAttribute::Type type,
	size_t count,
	unsigned buffer,
	size_t offset)
{
	orion_assert(!m_finalized);

	VertexAttribute attribute;
	attribute.semantic = semantic;
	attribute.index = index;
	attribute.type = type;
	attribute.count = count;
	attribute.buffer = buffer;
	attribute.offset = offset;

	const VertexBufferDesc *desc = this->buffer(buffer);

	orion_check(desc, "Attribute references unknown buffer %u", buffer);
	orion_check((offset + attribute.size()) <= desc->stride,
		"Attribute position exceeds buffer stride (offset: %u, size: %u, stride: %u)",
		offset, attribute.size(), desc->stride);
	orion_check(count >= 1 && count <= 4,
		"Unsupported attribute vector size %u", count);

	for(const VertexAttribute &exist : m_attributes) {
		orion_check(exist.semantic != semantic || exist.index != index,
			"Adding duplicate attribute (semantic: %d, index: %u)",
			semantic, index);
	}

	m_attributes.push_back(attribute);
}

/** Finalize the format descriptor. */
void VertexFormat::finalize() {
	finalize_impl();
	m_finalized = true;
}

/** Get a vertex buffer descriptor by index.
 * @param index		Buffer index.
 * @return		Pointer to buffer if set, null if not. */
const VertexBufferDesc *VertexFormat::buffer(unsigned index) const {
	return (index < m_buffers.size() && m_buffers[index].stride)
		? &m_buffers[index]
		: nullptr;
}

/** Find an attribute.
 * @param semantic	Attribute semantic.
 * @param index		Attribute index.
 * @return		Pointer to attribute if found, null if not. */
const VertexAttribute *VertexFormat::find_attribute(
	VertexAttribute::Semantic semantic,
	unsigned index) const
{
	for(const VertexAttribute &exist : m_attributes) {
		if(exist.semantic == semantic && exist.index == index)
			return &exist;
	}

	return nullptr;
}
