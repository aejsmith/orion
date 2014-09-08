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
	GLuint array() const { return m_array; }

	static bool mapAttribute(VertexAttribute::Semantic semantic, unsigned index, GLuint *gl);
protected:
	void finalizeImpl() override;
private:
	GLuint m_array;			/**< Vertex array object. */
	GPUBufferPtr m_boundIndices;	/**< Currently bound index buffer. */
};
