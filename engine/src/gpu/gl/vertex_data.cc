/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GL vertex data implementation.
 *
 * GL vertex array objects store the bindings of vertex attributes to source
 * buffers, attribute data format, and the element array buffer binding.
 * Except for the index buffer binding, this is the same as what VertexData
 * holds, therefore we can use VAOs to store the entire VertexData state.
 * Despite the index buffer not being held by VertexData we can additionally
 * make use of the VAO to store it - we hold a pointer to the last buffer used
 * with the VAO, and if the one being used for rendering is the same then we
 * don't rebind it.
 *
 * So that we don't tie a VAO to a specific program, we bind shader attributes
 * to attribute indices statically based on the semantic and index of the
 * attribute, rather than letting the linker assign attribute indices. This
 * allows us to use a single VAO with any program. GL implementations must
 * support a minimum of 16 attribute indices, so we divide up this space
 * between the various attribute semantics.
 */

#include "context.h"
#include "vertex_data.h"

/** Initialize the vertex data object.
 * @param vertices	Total number of vertices. */
GLVertexData::GLVertexData(size_t vertices) :
	VertexData(vertices),
	m_vao(0),
	m_bound_indices(nullptr)
{}

/** Destroy the vertex data object. */
GLVertexData::~GLVertexData() {
	if(m_vao)
		glDeleteVertexArrays(1, &m_vao);
}

/** Bind the VAO for rendering.
 * @param _indices	Index buffer being used for rendering. */
void GLVertexData::bind(GPUBufferPtr indices) {
	orion_assert(m_finalized);

	glBindVertexArray(m_vao);

	/*
	 * As described at the top of the file, if the index buffer being used
	 * for rendering is the same as the previous one used with this vertex
	 * data, we can avoid a call to glBindBuffer here.
	 *
	 * Don't need to do anything if there is no index buffer being used as
	 * in this case we will use glDrawArrays() over glDrawElements(), which
	 * ignores the index buffer binding.
	 */
	GLBuffer *buffer = static_cast<GLBuffer *>(indices.get());
	if(buffer) {
		if(unlikely(buffer != m_bound_indices)) {
			buffer->bind();
			m_bound_indices = buffer;
		}
	}
}

/** Bind the VAO for rendering. */
void GLVertexData::_finalize() {
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	for(const VertexAttribute &attribute : m_format->attributes()) {
		GLuint index;
		if(!map_attribute(attribute.semantic, attribute.index, &index)) {
			orion_abort(
				"GL: Cannot map attribute (semantic: %d, index: %u)",
				attribute.semantic, attribute.index);
		}

		/* FIXME: Check if type is supported. */
		GLenum type = gl::convert_attribute_type(attribute.type);
		void *offset = reinterpret_cast<void *>(static_cast<uintptr_t>(attribute.offset));

		const VertexBufferDesc *desc = m_format->buffer(attribute.buffer);
		const GLBuffer *buffer = static_cast<GLBuffer *>(m_buffers[attribute.buffer].get());
		buffer->bind();

		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, attribute.count, type, GL_FALSE, desc->stride, offset);
	}

	glBindVertexArray(g_gl_context->default_vao);
}

/** Map an attribute semantic/index to a GL attribute index.
 * @param semantic	Attribute semantic.
 * @param index		Attribute index.
 * @param gl		Where to store GL attribute index.
 * @return		Whether mapped successfully. */
bool GLVertexData::map_attribute(VertexAttribute::Semantic semantic, unsigned index, GLuint *gl) {
	/* TODO: Make use of all supported hardware attributes rather than the
	 * minimum of 16. Also, this is a somewhat arbitrary division for now,
	 * may need tweaking based on future requirements (e.g. probably don't
	 * need multiple positions). */
	switch(semantic) {
	case VertexAttribute::kPositionSemantic:
		if(index >= 2)
			return false;

		*gl = 0 + index;
		return true;
	case VertexAttribute::kNormalSemantic:
		if(index >= 2)
			return false;

		*gl = 2 + index;
		return true;
	case VertexAttribute::kTexCoordSemantic:
		if(index >= 10)
			return false;

		*gl = 4 + index;
		return true;
	case VertexAttribute::kDiffuseSemantic:
		if(index >= 1)
			return false;

		*gl = 14;
		return true;
	case VertexAttribute::kSpecularSemantic:
		if(index >= 1)
			return false;

		*gl = 15;
		return true;
	}

	return false;
}
