/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GPU interface class.
 */

#pragma once

#include "gpu/buffer.h"
#include "gpu/index_data.h"
#include "gpu/pipeline.h"
#include "gpu/shader.h"
#include "gpu/state.h"
#include "gpu/texture.h"
#include "gpu/vertex_data.h"

struct EngineConfiguration;

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

	/**
	 * Initialize the GPU interface.
	 *
	 * This function is called after the main window has been created to
	 * properly initialize the GPU interface. The constructor will be called
	 * before the main window is created so that any necessary SDL
	 * attributes can be configured.
	 */
	virtual void init() = 0;

	/**
	 * Resource creation methods.
	 */

	/** Create a GPU buffer.
	 * @see			GPUBuffer::GPUBuffer().
	 * @return		Pointer to created buffer. */
	virtual GPUBufferPtr createBuffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) = 0;

	/** Create an index data object.
	 * @see			GPUIndexData::GPUIndexData().
	 * @return		Pointer to created index data object. */
	virtual GPUIndexDataPtr createIndexData(GPUBuffer *buffer, GPUIndexData::Type type, size_t count);

	/** Create a pipeline object.
	 * @see			GPUPipeline::GPUPipeline().
	 * @return		Pointer to created pipeline. */
	virtual GPUPipelinePtr createPipeline(const GPUShaderArray &shaders);

	/** Create a sampler state object.
	 * @param desc		Descriptor for sampler state.
	 * @return		Pointer to created sampler state object. */
	virtual GPUSamplerStatePtr createSamplerState(const GPUSamplerStateDesc &desc) = 0;

	/** Create a 2D texture.
	* @see			GPUTexture::GPUTexture().
	 * @param desc		Descriptor containing texture parameters.
	 * @return		Pointer to created texture. */
	virtual GPUTexturePtr createTexture(const GPUTexture2DDesc &desc) = 0;

	/** Create a 2D array texture.
	* @see			GPUTexture::GPUTexture().
	 * @param desc		Descriptor containing texture parameters.
	 * @return		Pointer to created texture. */
	virtual GPUTexturePtr createTexture(const GPUTexture2DArrayDesc &desc) = 0;

	/** Create a cube texture.
	* @see			GPUTexture::GPUTexture().
	 * @param desc		Descriptor containing texture parameters.
	 * @return		Pointer to created texture. */
	virtual GPUTexturePtr createTexture(const GPUTextureCubeDesc &desc) = 0;

	/** Create a 3D texture.
	* @see			GPUTexture::GPUTexture().
	 * @param desc		Descriptor containing texture parameters.
	 * @return		Pointer to created texture. */
	virtual GPUTexturePtr createTexture(const GPUTexture3DDesc &desc) = 0;

	/** Create a vertex data object.
	 * @see			GPUVertexData::GPUVertexData().
	 * @return		Pointer to created vertex data object. */
	virtual GPUVertexDataPtr createVertexData(size_t count, GPUVertexFormat *format, GPUBufferArray &buffers);

	/** Create a vertex format.
	 * @see			GPUVertexFormat::GPUVertexFormat(). */
	virtual GPUVertexFormatPtr createVertexFormat(VertexBufferLayoutArray &buffers, VertexAttributeArray &attributes);

	/**
	 * Shader methods.
	 */

	/** Compile a GPU shader from GLSL source.
	 * @note		In future I expect that this will exist only for
	 * 			non-cooked builds (it may possibly even be moved
	 *			out of GPUInterface). It would return a blob
	 *			that can be passed to a different method. For
	 *			GL this would be just the GLSL source with some
	 *			preprocessing done, but for D3D and other APIs
	 *			we can return a compiled blob. This would then
	 *			be stored in the cooked Shader asset.
	 * @param type		Type of the shader.
	 * @param source	Shader source string.
	 * @return		Pointer to created shader, null if compilation
	 *			fails. */
	virtual GPUShaderPtr compileShader(GPUShader::Type type, const std::string &source) = 0;

	/**
	 * State methods.
	 */

	/** Bind a pipeline for rendering.
	 * @param pipeline	Pipeline to use. */
	virtual void bindPipeline(GPUPipeline *pipeline) = 0;

	/** Bind a texture.
	 * @param index		Texture unit index to bind to.
	 * @param texture	Texture to bind.
	 * @param sampler	Sampler state. */
	virtual void bindTexture(unsigned index, GPUTexture *texture, GPUSamplerState *sampler) = 0;

	/** Bind a uniform buffer.
	 * @param index		Uniform block index to bind to.
	 * @param buffer	Buffer to bind. */
	virtual void bindUniformBuffer(unsigned index, GPUBuffer *buffer) = 0;

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
	 * Set the render targets.
	 *
	 * Sets the current render target. A render target is described by a
	 * GPURenderTargetDesc, which specifies a number of texture colour
	 * targets and a depth/stencil target. If the descriptor pointer is
	 * given as null, the render targeet will be set to the main window.
	 * This function will reset the viewport to the size of the new render
	 * target.
	 *
	 * @param target	Pointer to render target descriptor.
	 */
	virtual void setRenderTarget(const GPURenderTargetDesc *target) = 0;

	/** Set the viewport.
	 * @param viewport	Viewport rectangle in pixels. Must be <= size of
	 *			the current render target. */
	virtual void setViewport(const IntRect &viewport) = 0;

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
	virtual void draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) = 0;
protected:
	GPUInterface() {}
};

extern EngineGlobal<GPUInterface> g_gpu;
