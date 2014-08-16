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

/** Set the blending mode.
 * @param func		Blending function.
 * @param source_factor	Source blend factor.
 * @param dest_factor	Destination factor. */
void GLGPUInterface::set_blend_mode(BlendFunc func, BlendFactor source_factor, BlendFactor dest_factor) {
	bool enable_blend =
		func != BlendFunc::kAdd ||
		source_factor != BlendFactor::kOne ||
		dest_factor != BlendFactor::kZero;

	g_gl_context->state.enable_blend(enable_blend);
	g_gl_context->state.set_blend_equation(gl::convert_blend_func(func));
	g_gl_context->state.set_blend_func(
		gl::convert_blend_factor(source_factor),
		gl::convert_blend_factor(dest_factor));
}

/** Set the depth testing mode.
 * @param func		Depth comparison function.
 * @param enable_write	Whether to enable depth writes. */
void GLGPUInterface::set_depth_mode(ComparisonFunc func, bool enable_write) {
	/* Documentation for glDepthFunc: "Even if the depth buffer exists and
	 * the depth mask is non-zero, the depth buffer is not updated if the
	 * depth test is disabled". */
	bool enable_test = func != ComparisonFunc::kAlways || enable_write;

	g_gl_context->state.enable_depth_test(enable_test);
	g_gl_context->state.enable_depth_write(enable_write);
	g_gl_context->state.set_depth_func(gl::convert_comparison_func(func));
}

/** End a frame and present it on screen.
 * @param vsync		Whether to wait for vertical sync. */
void GLGPUInterface::end_frame(bool vsync) {
	g_gl_context->state.set_swap_interval(vsync);
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
