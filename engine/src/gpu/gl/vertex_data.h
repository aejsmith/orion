/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GL vertex data implementation.
 */

#pragma once

#include "buffer.h"

/** OpenGL vertex data implementation. */
class GLVertexData : public VertexData {
public:
	explicit GLVertexData(size_t vertices);
	~GLVertexData();

	void bind(const GPUBufferPtr &indices);

	/** Get the VAO ID.
	 * @return		VAO ID. */
	GLuint vao() const { return m_vao; }

	static bool map_attribute(VertexAttribute::Semantic semantic, unsigned index, GLuint *gl);
protected:
	void finalize_impl() override;
private:
	GLuint m_vao;			/**< Vertex array object. */
	GPUBufferPtr m_bound_indices;	/**< Currently bound index buffer. */
};
