/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GL vertex data implementation.
 */

#ifndef ORION_GPU_GL_VERTEX_DATA_H
#define ORION_GPU_GL_VERTEX_DATA_H

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
	void _finalize();
private:
	GLuint m_vao;			/**< Vertex array object. */
	GPUBufferPtr m_bound_indices;	/**< Currently bound index buffer. */
};

#endif /* ORION_GPU_GL_VERTEX_DATA_H */
