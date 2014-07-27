/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL GPU interface implementation.
 */

#include "buffer.h"
#include "context.h"
#include "gpu.h"
#include "pipeline.h"
#include "program.h"
#include "vertex_data.h"

/** Initialize the GPU interface.
 * @param config	Engine configuration. */
GLGPUInterface::GLGPUInterface(const EngineConfiguration &config) {
	g_gl_context = new GLContext(config);
}

/** Shut down the GPU interface. */
GLGPUInterface::~GLGPUInterface() {
	delete g_gl_context;
}

/**
 * Object creation methods.
 */

/** Create a GPU buffer.
 * @see		GPUBuffer::GPUBuffer().
 * @return	Pointer to created vertex buffer. */
GPUBufferPtr GLGPUInterface::create_buffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) {
	GPUBuffer *buffer = new GLBuffer(type, usage, size);
	return GPUBufferPtr(buffer);
}

/** Create a vertex data object.
 * @see			VertexData::VertexData().
 * @return		Pointer to created vertex data object. */
VertexDataPtr GLGPUInterface::create_vertex_data(size_t vertices) {
	VertexData *data = new GLVertexData(vertices);
	return VertexDataPtr(data);
}

/** Create a pipeline object.
 * @return		Pointer to created pipeline. */
GPUPipelinePtr GLGPUInterface::create_pipeline() {
	GPUPipeline *pipeline = new GLPipeline();
	return GPUPipelinePtr(pipeline);
}

/** Load a GPU program.
 * @param path		Path to the program source.
 * @param type		Type of the program.
 * @return		Pointer to created program. */
GPUProgramPtr GLGPUInterface::load_program(const char *path, GPUProgram::Type type) {
	GPUProgram *program = new GLProgram(path, type);
	return GPUProgramPtr(program);
}

/**
 * Rendering methods.
 */

/** Swap buffers. */
void GLGPUInterface::swap_buffers() {
	SDL_GL_SwapWindow(g_gl_context->sdl_window);
}

/** Clear rendering buffers.
 * @param buffers	Buffers to clear (bitmask of RenderBuffer values).
 * @param colour	Colour to clear to.
 * @param depth		Depth value to clear to.
 * @param stencil	Stencil value to clear to. */
void GLGPUInterface::clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil) {
	GLbitfield mask = 0;

	if(buffers & RenderBuffer::kColourBuffer) {
		g_gl_context->state.set_clear_colour(colour);
		mask |= GL_COLOR_BUFFER_BIT;
	}

	if(buffers & RenderBuffer::kDepthBuffer) {
		g_gl_context->state.set_clear_depth(depth);
		mask |= GL_DEPTH_BUFFER_BIT;
	}

	if(buffers & RenderBuffer::kStencilBuffer) {
		g_gl_context->state.set_clear_stencil(stencil);
		mask |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(mask);
}

/** Bind a pipeline for rendering.
 * @param _pipeline	Pipeline to use. */
void GLGPUInterface::bind_pipeline(const GPUPipelinePtr &_pipeline) {
	GLPipeline *pipeline = static_cast<GLPipeline *>(_pipeline.get());
	pipeline->bind();
}

/** Bind a uniform buffer.
 * @param index		Uniform block index to bind to.
 * @param _buffer	Buffer to bind. */
void GLGPUInterface::bind_uniform_buffer(unsigned index, const GPUBufferPtr &_buffer) {
	GLBuffer *buffer = static_cast<GLBuffer *>(_buffer.get());
	orion_assert(buffer->type() == GPUBuffer::kUniformBuffer);

	buffer->bind_indexed(index);
}

/** Draw primitives.
 * @param type		Primitive type to render.
 * @param _vertices	Vertex data to use.
 * @param indices	Index data to use (can be null). */
void GLGPUInterface::draw(PrimitiveType type, const VertexDataPtr &_vertices, const IndexDataPtr &indices) {
	GLVertexData *vertices = static_cast<GLVertexData *>(_vertices.get());

	/* Bind the VAO and the index buffer (if any). */
	vertices->bind((indices) ? indices->buffer() : nullptr);

	GLenum mode = gl::convert_primitive_type(type);
	if(indices) {
		/* FIXME: Check whether index type is supported (in generic
		 * code?) */
		glDrawElements(mode, indices->count(), gl::convert_index_type(indices->type()), nullptr);
	} else {
		glDrawArrays(mode, 0, vertices->count());
	}
}
