/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL GPU interface implementation.
 */

#ifndef ORION_GPU_GL_GPU_H
#define ORION_GPU_GL_GPU_H

#include "defs.h"

/** OpenGL GPU interface implementation. */
class GLGPUInterface : public GPUInterface {
public:
	GLGPUInterface(const EngineConfiguration &config);
	~GLGPUInterface();

	GPUBufferPtr create_buffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size);
	VertexDataPtr create_vertex_data(size_t vertices);
	GPUPipelinePtr create_pipeline();
	GPUProgramPtr load_program(const char *path, GPUProgram::Type type);

	void swap_buffers();
	void clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil);
	void bind_pipeline(const GPUPipelinePtr &pipeline);
	void draw(PrimitiveType type, const VertexDataPtr &vertices, const IndexDataPtr &indices);
};

#endif /* ORION_GPU_GL_GPU_H */
