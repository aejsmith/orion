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
	GLGPUInterface();
	~GLGPUInterface();

	void init(SDL_Window *window);

	GPUBufferPtr create_buffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size);
	VertexDataPtr create_vertex_data(size_t vertices);
	GPUPipelinePtr create_pipeline();
	GPUProgramPtr load_program(const char *path, GPUProgram::Type type);

	void bind_pipeline(const GPUPipelinePtr &pipeline);
	void bind_uniform_buffer(unsigned index, const GPUBufferPtr &buffer);
	void set_blend_mode(BlendFunc func, BlendFactor source_factor, BlendFactor dest_factor);
	void set_depth_mode(ComparisonFunc func, bool enable_write);

	void end_frame(bool vsync);

	void clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil);
	void draw(PrimitiveType type, const VertexDataPtr &vertices, const IndexDataPtr &indices);
};

#endif /* ORION_GPU_GL_GPU_H */
