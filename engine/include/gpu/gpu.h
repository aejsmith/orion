/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GPU interface class.
 */

#ifndef ORION_GPU_GPU_H
#define ORION_GPU_GPU_H

#include "core/engine.h"

#include "gpu/buffer.h"
#include "gpu/defs.h"
#include "gpu/index_data.h"
#include "gpu/pipeline.h"
#include "gpu/program.h"
#include "gpu/vertex_data.h"

struct SDL_Window;

/**
 * Low-level GPU interface.
 *
 * The purpose of this interface is to provide a low level interface for
 * accessing the GPU. It wraps the graphics API in use (GL, D3D, etc.) and
 * provides an interface on top of that which the high level renderer can use
 * without having to care about the API in use.
 */
class GPUInterface : Noncopyable {
public:
	static GPUInterface *create(const EngineConfiguration &config);

	virtual ~GPUInterface() {}

	/** Initialize the GPU interface.
	 * @param window	Created SDL window. */
	virtual void init(SDL_Window *window) = 0;

	/**
	 * Object creation methods.
	 */

	/** Create a GPU buffer.
	 * @see			GPUBuffer::GPUBuffer().
	 * @return		Pointer to created buffer. */
	virtual GPUBufferPtr create_buffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) = 0;

	virtual VertexFormatPtr create_vertex_format();
	virtual VertexDataPtr create_vertex_data(size_t vertices);
	virtual IndexDataPtr create_index_data(const GPUBufferPtr &buffer, IndexData::Type type, size_t count);
	virtual GPUPipelinePtr create_pipeline();

	/** Load a GPU program.
	 * @note	 	This is temporary until the resource system is
	 *			implemented.
	 * @param path		Path to the program source.
	 * @param type		Type of the program.
	 * @return		Pointer to created program. */
	virtual GPUProgramPtr load_program(const char *path, GPUProgram::Type type) = 0;

	/**
	 * Rendering methods.
	 */

	/** Clear rendering buffers.
	 * @param buffers	Buffers to clear (bitmask of RenderBuffer values).
	 * @param colour	Colour to clear to.
	 * @param depth		Depth value to clear to.
	 * @param stencil	Stencil value to clear to. */
	virtual void clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil) = 0;

	/** Bind a pipeline for rendering.
	 * @param pipeline	Pipeline to use. */
	virtual void bind_pipeline(const GPUPipelinePtr &pipeline) = 0;

	/** Bind a uniform buffer.
	 * @param index		Uniform block index to bind to.
	 * @param buffer	Buffer to bind. */
	virtual void bind_uniform_buffer(unsigned index, const GPUBufferPtr &buffer) = 0;

	/** Draw primitives.
	 * @param type		Primitive type to render.
	 * @param vertices	Vertex data to use.
	 * @param indices	Index data to use (can be null). */
	virtual void draw(PrimitiveType type, const VertexDataPtr &vertices, const IndexDataPtr &indices) = 0;

	/** End a frame and present it on screen.
	 * @param vsync		Whether to wait for vertical sync. */
	virtual void end_frame(bool vsync) {}
protected:
	GPUInterface() {}
};

#endif /* ORION_GPU_GPU_H */
