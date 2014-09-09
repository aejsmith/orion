/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL GPU interface implementation.
 */

#include "buffer.h"
#include "gl.h"
#include "pipeline.h"
#include "program.h"
#include "texture.h"
#include "vertex_data.h"

#include "engine/window.h"

/** Bind a pipeline for rendering.
 * @param _pipeline	Pipeline to use. */
void GLGPUInterface::bindPipeline(const GPUPipelinePtr &_pipeline) {
	GLPipeline *pipeline = static_cast<GLPipeline *>(_pipeline.get());
	pipeline->bind();
}

/** Bind a texture.
 * @param index		Texture unit index to bind to.
 * @param _texture	Texture to bind. */
void GLGPUInterface::bindTexture(unsigned index, const GPUTexturePtr &_texture) {
	GLTexture *texture = static_cast<GLTexture *>(_texture.get());
	texture->bind(index);
}

/** Bind a uniform buffer.
 * @param index		Uniform block index to bind to.
 * @param _buffer	Buffer to bind. */
void GLGPUInterface::bindUniformBuffer(unsigned index, const GPUBufferPtr &_buffer) {
	GLBuffer *buffer = static_cast<GLBuffer *>(_buffer.get());
	orionAssert(buffer->type() == GPUBuffer::kUniformBuffer);

	buffer->bindIndexed(index);
}

/** Set the blending mode.
 * @param func		Blending function.
 * @param sourceFactor	Source blend factor.
 * @param destFactor	Destination factor. */
void GLGPUInterface::setBlendMode(BlendFunc func, BlendFactor sourceFactor, BlendFactor destFactor) {
	bool enableBlend =
		func != BlendFunc::kAdd ||
		sourceFactor != BlendFactor::kOne ||
		destFactor != BlendFactor::kZero;

	g_opengl->state.enableBlend(enableBlend);
	g_opengl->state.setBlendEquation(gl::convertBlendFunc(func));
	g_opengl->state.setBlendFunc(
		gl::convertBlendFactor(sourceFactor),
		gl::convertBlendFactor(destFactor));
}

/** Set the depth testing mode.
 * @param func		Depth comparison function.
 * @param enableWrite	Whether to enable depth writes. */
void GLGPUInterface::setDepthMode(ComparisonFunc func, bool enableWrite) {
	/* Documentation for glDepthFunc: "Even if the depth buffer exists and
	 * the depth mask is non-zero, the depth buffer is not updated if the
	 * depth test is disabled". */
	bool enableTest = func != ComparisonFunc::kAlways || enableWrite;

	g_opengl->state.enableDepthTest(enableTest);
	g_opengl->state.enableDepthWrite(enableWrite);
	g_opengl->state.setDepthFunc(gl::convertComparisonFunc(func));
}

/** End a frame and present it on screen.
 * @param vsync		Whether to wait for vertical sync. */
void GLGPUInterface::endFrame(bool vsync) {
	g_opengl->state.setSwapInterval(vsync);
	SDL_GL_SwapWindow(g_mainWindow->sdlWindow());
}

/** Clear rendering buffers.
 * @param buffers	Buffers to clear (bitmask of RenderBuffer values).
 * @param colour	Colour to clear to.
 * @param depth		Depth value to clear to.
 * @param stencil	Stencil value to clear to. */
void GLGPUInterface::clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil) {
	GLbitfield mask = 0;

	if(buffers & RenderBuffer::kColourBuffer) {
		g_opengl->state.setClearColour(colour);
		mask |= GL_COLOR_BUFFER_BIT;
	}

	if(buffers & RenderBuffer::kDepthBuffer) {
		g_opengl->state.setClearDepth(depth);
		mask |= GL_DEPTH_BUFFER_BIT;
	}

	if(buffers & RenderBuffer::kStencilBuffer) {
		g_opengl->state.setClearStencil(stencil);
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

	GLenum mode = gl::convertPrimitiveType(type);
	if(indices) {
		/* FIXME: Check whether index type is supported (in generic
		 * code?) */
		glDrawElements(mode, indices->count(), gl::convertIndexType(indices->type()), nullptr);
	} else {
		glDrawArrays(mode, 0, vertices->count());
	}
}
