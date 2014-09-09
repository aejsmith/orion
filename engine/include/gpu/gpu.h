/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GPU interface class.
 */

#pragma once

#include "gpu/buffer.h"
#include "gpu/index_data.h"
#include "gpu/pipeline.h"
#include "gpu/program.h"
#include "gpu/texture.h"
#include "gpu/vertex_data.h"

struct EngineConfiguration;
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
	virtual GPUBufferPtr createBuffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) = 0;

	/** Create a vertex format descriptor.
	 * @return		Pointer to created vertex format descriptor. */
	virtual VertexFormatPtr createVertexFormat() {
		return VertexFormatPtr(new VertexFormat);
	}

	/** Create a vertex data object.
	 * @see			VertexData::VertexData().
	 * @return		Pointer to created vertex data object. */
	virtual VertexDataPtr createVertexData(size_t vertices) {
		return VertexDataPtr(new VertexData(vertices));
	}

	/** Create an index data object.
	 * @see			IndexData::IndexData().
	 * @return		Pointer to created index data object. */
	virtual IndexDataPtr createIndexData(const GPUBufferPtr &buffer, IndexData::Type type, size_t count) {
		return IndexDataPtr(new IndexData(buffer, type, count));
	}

	/** Create a pipeline object.
	 * @return		Pointer to created pipeline. */
	virtual GPUPipelinePtr createPipeline() {
		return GPUPipelinePtr(new GPUPipeline());
	}

	/** Load a GPU program.
	 * @note	 	This is temporary until the resource system is
	 *			implemented.
	 * @param path		Path to the program source.
	 * @param type		Type of the program.
	 * @return		Pointer to created program. */
	virtual GPUProgramPtr loadProgram(const char *path, GPUProgram::Type type) = 0;

	/** Create a 2D texture.
	 * @param desc		Descriptor containing texture parameters.
	 * @return		Pointer to created texture. */
	virtual GPUTexturePtr createTexture(const GPUTexture2DDesc &desc) = 0;

	/** Create a 2D array texture.
	 * @param desc		Descriptor containing texture parameters.
	 * @return		Pointer to created texture. */
	virtual GPUTexturePtr createTexture(const GPUTexture2DArrayDesc &desc) = 0;

	/** Create a cube texture.
	 * @param desc		Descriptor containing texture parameters.
	 * @return		Pointer to created texture. */
	virtual GPUTexturePtr createTexture(const GPUTextureCubeDesc &desc) = 0;

	/** Create a 3D texture.
	 * @param desc		Descriptor containing texture parameters.
	 * @return		Pointer to created texture. */
	virtual GPUTexturePtr createTexture(const GPUTexture3DDesc &desc) = 0;

	/**
	 * State methods.
	 */

	/** Bind a pipeline for rendering.
	 * @param pipeline	Pipeline to use. */
	virtual void bindPipeline(const GPUPipelinePtr &pipeline) = 0;

	/** Bind a texture.
	 * @param index		Texture unit index to bind to.
	 * @param texture	Texture to bind. */
	virtual void bindTexture(unsigned index, const GPUTexturePtr &texture) = 0;

	/** Bind a uniform buffer.
	 * @param index		Uniform block index to bind to.
	 * @param buffer	Buffer to bind. */
	virtual void bindUniformBuffer(unsigned index, const GPUBufferPtr &buffer) = 0;

	/**
	 * Set the blending mode.
	 *
	 * Sets the colour blending mode, which determines how fragment colour
	 * outputs are combined with the values already in the colour buffers.
	 * The default arguments set the blend mode to add, the source factor
	 * to one, and the destination factor to zero. This effectively disables
	 * blending.
	 *
	 * @param func		Blending function.
	 * @param sourceFactor	Source blend factor.
	 * @param destFactor	Destination factor.
	 */
	virtual void setBlendMode(
		BlendFunc func = BlendFunc::kAdd,
		BlendFactor sourceFactor = BlendFactor::kOne,
		BlendFactor destFactor = BlendFactor::kZero) = 0;

	/**
	 * Set the depth testing mode.
	 *
	 * Sets the depth testing mode. The default arguments set the function
	 * to less than or equal, and enables writes to the depth buffer.
	 *
	 * @param func		Depth comparison function.
	 * @param enableWrite	Whether to enable depth writes.
	 */
	virtual void setDepthMode(
		ComparisonFunc func = ComparisonFunc::kLessOrEqual,
		bool enableWrite = true) = 0;

	/**
	 * Frame methods.
	 */

	/** End a frame and present it on screen.
	 * @param vsync		Whether to wait for vertical sync. */
	virtual void endFrame(bool vsync) {}

	/**
	 * Rendering methods.
	 */

	/** Clear rendering buffers.
	 * @param buffers	Buffers to clear (bitmask of ClearBuffer values).
	 * @param colour	Colour to clear to.
	 * @param depth		Depth value to clear to.
	 * @param stencil	Stencil value to clear to. */
	virtual void clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil) = 0;

	/** Draw primitives.
	 * @param type		Primitive type to render.
	 * @param vertices	Vertex data to use.
	 * @param indices	Index data to use (can be null). */
	virtual void draw(PrimitiveType type, const VertexDataPtr &vertices, const IndexDataPtr &indices) = 0;
protected:
	GPUInterface() {}
};

extern EngineGlobal<GPUInterface> g_gpu;
