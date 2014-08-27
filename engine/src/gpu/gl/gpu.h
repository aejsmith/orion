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

	void init(SDL_Window *window) override;

	GPUBufferPtr create_buffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) override;
	VertexDataPtr create_vertex_data(size_t vertices) override;
	GPUPipelinePtr create_pipeline() override;
	GPUProgramPtr load_program(const char *path, GPUProgram::Type type) override;

	void bind_pipeline(const GPUPipelinePtr &pipeline) override;
	void bind_uniform_buffer(unsigned index, const GPUBufferPtr &buffer) override;
	void set_blend_mode(BlendFunc func, BlendFactor source_factor, BlendFactor dest_factor) override;
	void set_depth_mode(ComparisonFunc func, bool enable_write) override;

	void end_frame(bool vsync) override;

	void clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil) override;
	void draw(PrimitiveType type, const VertexDataPtr &vertices, const IndexDataPtr &indices) override;
};

#endif /* ORION_GPU_GL_GPU_H */
