/*
 * Copyright (C) 2015-2016 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               GPU manager class.
 */

#pragma once

#include "gpu/buffer.h"
#include "gpu/index_data.h"
#include "gpu/pipeline.h"
#include "gpu/program.h"
#include "gpu/state.h"
#include "gpu/texture.h"
#include "gpu/vertex_data.h"

struct EngineConfiguration;

class Window;

/**
 * Low-level GPU interface.
 *
 * The purpose of this class is to provide a low level interface for accessing
 * the GPU. It wraps the graphics API in use (GL, D3D, etc.) and provides an
 * interface on top of that which the high level renderer can use without having
 * to care about the API in use.
 */
class GPUManager : Noncopyable {
public:
    static GPUManager *create(const EngineConfiguration &config, Window *&window);

    virtual ~GPUManager() {}

    /**
     * Resource creation methods.
     */

    /** Create a blend state object.
     * @param desc          Descriptor for blend state.
     * @return              Created blend state object. */
    virtual GPUBlendStatePtr createBlendState(const GPUBlendStateDesc &desc) = 0;

    /** Create a GPU buffer.
     * @see                 GPUBuffer::GPUBuffer().
     * @return              Pointer to created buffer. */
    virtual GPUBufferPtr createBuffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) = 0;

    /** Create a depth/stencil state object.
     * @param desc          Descriptor for depth/stencil state.
     * @return              Created depth/stencil state object. */
    virtual GPUDepthStencilStatePtr createDepthStencilState(const GPUDepthStencilStateDesc &desc) = 0;

    /** Create an index data object.
     * @see                 GPUIndexData::GPUIndexData().
     * @return              Pointer to created index data object. */
    virtual GPUIndexDataPtr createIndexData(
        GPUBuffer *buffer,
        GPUIndexData::Type type,
        size_t count,
        size_t offset = 0);

    /** Create a pipeline object.
     * @see                 GPUPipeline::GPUPipeline().
     * @return              Pointer to created pipeline. */
    virtual GPUPipelinePtr createPipeline(const GPUPipelineDesc &desc);

    /** Create a rasterizer state object.
     * @param desc          Descriptor for rasterizer state.
     * @return              Created rasterizer state object. */
    virtual GPURasterizerStatePtr createRasterizerState(const GPURasterizerStateDesc &desc) = 0;

    /** Create a sampler state object.
     * @param desc          Descriptor for sampler state.
     * @return              Pointer to created sampler state object. */
    virtual GPUSamplerStatePtr createSamplerState(const GPUSamplerStateDesc &desc) = 0;

    /** Create a texture.
     * @see                 GPUTexture::GPUTexture().
     * @param desc          Descriptor containing texture parameters.
     * @return              Pointer to created texture. */
    virtual GPUTexturePtr createTexture(const GPUTextureDesc &desc) = 0;

    /** Create a texture view.
     * @param image         Image to create the view for.
     * @return              Pointer to created texture view. */
    // TODO: GPUTextureImageRef doesn't expose all functionality but it works for now.
    virtual GPUTexturePtr createTextureView(const GPUTextureImageRef &image) = 0;

    /** Create a vertex data object.
     * @see                 GPUVertexData::GPUVertexData().
     * @return              Pointer to created vertex data object. */
    virtual GPUVertexDataPtr createVertexData(size_t count, GPUVertexFormat *format, GPUBufferArray &buffers);

    /** Create a vertex format.
     * @see                 GPUVertexFormat::GPUVertexFormat(). */
    virtual GPUVertexFormatPtr createVertexFormat(VertexBufferLayoutArray &buffers, VertexAttributeArray &attributes);

    /**
     * Shader methods.
     */

    /** Compile a GPU program from GLSL source.
     * @note                In future I expect that this will exist only for
     *                      non-cooked builds (it may possibly even be moved
     *                      out of GPUManager). It would return a blob that
     *                      can be passed to a different method. For GL this
     *                      would be just the GLSL source with some pre-
     *                      processing done, but for D3D and other APIs we can
     *                      return a compiled blob. This would then be stored in
     *                      the cooked Shader asset.
     * @param stage         Stage that the program is for.
     * @param source        Shader source string.
     * @return              Pointer to created shader, null if compilation fails. */
    virtual GPUProgramPtr compileProgram(unsigned stage, const std::string &source) = 0;

    /**
     * State methods.
     */

    /** Bind a pipeline for rendering.
     * @param pipeline      Pipeline to use. */
    virtual void bindPipeline(GPUPipeline *pipeline) = 0;

    /** Bind a texture.
     * @param index         Texture unit index to bind to.
     * @param texture       Texture to bind.
     * @param sampler       Sampler state. */
    virtual void bindTexture(unsigned index, GPUTexture *texture, GPUSamplerState *sampler) = 0;

    /** Bind a uniform buffer.
     * @param index         Uniform block index to bind to.
     * @param buffer        Buffer to bind. */
    virtual void bindUniformBuffer(unsigned index, GPUBuffer *buffer) = 0;

    /** Set the blend state.
     * @param state         Blend state to set. */
    virtual void setBlendState(GPUBlendState *state) = 0;

    /** Set the depth/stencil state.
     * @param state         Depth/stencil state to set. */
    virtual void setDepthStencilState(GPUDepthStencilState *state) = 0;

    /** Set the rasterizer state.
     * @param state         Rasterizer state to set. */
    virtual void setRasterizerState(GPURasterizerState *state) = 0;

    /**
     * Set the render targets.
     *
     * Sets the current render target. A render target is described by a
     * GPURenderTargetDesc, which specifies a number of texture colour targets
     * and a depth/stencil target. If the descriptor pointer is given as null,
     * the render target will be set to the main window. If a viewport rectangle
     * is provided, that viewport will be set, else this function will reset the
     * viewport to the size of the new render target.
     *
     * @param desc          Pointer to render target descriptor.
     * @param viewport      Optional viewport rectangle.
     */
    virtual void setRenderTarget(const GPURenderTargetDesc *desc, const IntRect *viewport = nullptr) = 0;

    /** Set the viewport.
     * @param viewport      Viewport rectangle in pixels. Must be <= size of
     *                      the current render target. */
    virtual void setViewport(const IntRect &viewport) = 0;

    /** Set the scissor test parameters.
     * @param enable        Whether to enable scissor testing.
     * @param scissor       Scissor rectangle. */
    virtual void setScissor(bool enable, const IntRect &scissor) = 0;

    /**
     * Frame methods.
     */

    /** End a frame and present it on screen.
     * @param vsync         Whether to wait for vertical sync. */
    virtual void endFrame(bool vsync) {}

    /**
     * Rendering methods.
     */

    /**
     * Copy pixels from one texture to another.
     *
     * Copies a rectangle of pixels from one texture to another. If either the
     * source or dest arguments are null image references, the main window will
     * be used.
     *
     * @param source        Source texture image reference.
     * @param dest          Destination texture image reference.
     * @param sourcePos     Position in source texture to copy from.
     * @param destPos       Position in destination texture to copy to.
     * @param size          Size of area to copy.
     */
    virtual void blit(
        const GPUTextureImageRef &source,
        const GPUTextureImageRef &dest,
        glm::ivec2 sourcePos,
        glm::ivec2 destPos,
        glm::ivec2 size) = 0;

    /** Clear rendering buffers.
     * @param buffers       Buffers to clear (bitmask of ClearBuffer values).
     * @param colour        Colour to clear to.
     * @param depth         Depth value to clear to.
     * @param stencil       Stencil value to clear to. */
    virtual void clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil) = 0;

    /** Draw primitives.
     * @param type          Primitive type to render.
     * @param vertices      Vertex data to use.
     * @param indices       Index data to use (can be null). */
    virtual void draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) = 0;

    /**
     * Constant state methods.
     *
     * These methods are used to set GPU state to constant values known at
     * compile time. They are a shortcut which avoids a hash lookup for a
     * matching state object at every call.
     */

    template <
        BlendFunc func = BlendFunc::kAdd,
        BlendFactor sourceFactor = BlendFactor::kOne,
        BlendFactor destFactor = BlendFactor::kZero>
    void setBlendState();

    template <
        ComparisonFunc depthFunc = ComparisonFunc::kLessOrEqual,
        bool depthWrite = true>
    void setDepthStencilState();

    template <
        CullMode cullMode = CullMode::kBack,
        bool depthClamp = false>
    void setRasterizerState();
protected:
    GPUManager() {}
};

extern GPUManager *g_gpuManager;

#include "gpu/const_state.h"
