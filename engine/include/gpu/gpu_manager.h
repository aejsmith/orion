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

#include "core/hash_table.h"
#include "core/string.h"

#include "gpu/buffer.h"
#include "gpu/index_data.h"
#include "gpu/pipeline.h"
#include "gpu/program.h"
#include "gpu/render_pass.h"
#include "gpu/resource.h"
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

    virtual ~GPUManager();

    /**
     * Object creation methods.
     */

    /** Create a GPU buffer.
     * @param desc          Descriptor for the buffer.
     * @return              Pointer to created buffer. */
    virtual GPUBufferPtr createBuffer(const GPUBufferDesc &desc) = 0;

    virtual GPUIndexDataPtr createIndexData(
        GPUBuffer *buffer,
        GPUIndexData::Type type,
        size_t count,
        size_t offset = 0);

    /** Create a pipeline object.
     * @param desc          Parameters for the pipeline.
     * @return              Pointer to created pipeline. */
    virtual GPUPipelinePtr createPipeline(GPUPipelineDesc &&desc) = 0;

    virtual GPURenderPassPtr createRenderPass(GPURenderPassDesc &&desc);

    /** Create a texture.
     * @param desc          Descriptor containing texture parameters.
     * @return              Pointer to created texture. */
    virtual GPUTexturePtr createTexture(const GPUTextureDesc &desc) = 0;

    /** Create a texture view.
     * @param image         Image to create the view for.
     * @return              Pointer to created texture view. */
    // TODO: GPUTextureImageRef doesn't expose all functionality but it works for now.
    virtual GPUTexturePtr createTextureView(const GPUTextureImageRef &image) = 0;

    virtual GPUVertexDataLayoutPtr createVertexDataLayout(GPUVertexDataLayoutDesc &&desc);
    virtual GPUVertexDataPtr createVertexData(
        size_t count,
        GPUVertexDataLayout *layout,
        GPUBufferArray &&buffers);

    /**
     * State object methods.
     */

    GPUBlendStatePtr getBlendState(const GPUBlendStateDesc &desc);
    GPUDepthStencilStatePtr getDepthStencilState(const GPUDepthStencilStateDesc &desc);
    GPURasterizerStatePtr getRasterizerState(const GPURasterizerStateDesc &desc);
    GPUSamplerStatePtr getSamplerState(const GPUSamplerStateDesc &desc);

    /**
     * Shader methods.
     */

    virtual GPUResourceSetLayoutPtr createResourceSetLayout(GPUResourceSetLayoutDesc &&desc);
    virtual GPUResourceSetPtr createResourceSet(GPUResourceSetLayout *layout);

    /** Create a GPU program from a SPIR-V binary.
     * @param stage         Stage that the program is for.
     * @param spirv         SPIR-V binary for the shader.
     * @param name          Name of the shader for debugging purposes.
     * @return              Pointer to created shader on success, null on error. */
    virtual GPUProgramPtr createProgram(
        unsigned stage,
        const std::vector<uint32_t> &spirv,
        const std::string &name) = 0;

    /**
     * Frame methods.
     */

    /** End a frame and present it on screen. */
    virtual void endFrame() = 0;

    /**
     * Texture operations.
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

    /**
     * Rendering methods.
     */

    /**
     * Begin a render pass.
     *
     * Begins a new render pass instance. The render pass defines the targets
     * that will be drawn to. All draw calls must take place within a render
     * pass. Once the render pass is finished, it must be ended by calling
     * endRenderPass().
     *
     * Beginning a render pass resets several pieces of state: the viewport
     * will be set to the specified render area, the scissor test will be
     * disabled, and the blend, depth/stencil and rasterizer states will be
     * set to the default states.
     *
     * @param desc          Descriptor for the render pass instance.
     */
    virtual void beginRenderPass(const GPURenderPassInstanceDesc &desc) = 0;

    /** End the current render pass. */
    virtual void endRenderPass() = 0;

    /** Bind a pipeline for rendering.
     * @param pipeline      Pipeline to use. */
    virtual void bindPipeline(GPUPipeline *pipeline) = 0;

    /**
     * Bind a resource set.
     *
     * Binds the specified resource set to a set index for upcoming draws. Note
     * that after binding a resource set with this function, it must not be
     * changed for the remainder of the frame.
     *
     * @param index         Resource set index to bind to.
     * @param resources     Resource set to bind.
     */
    virtual void bindResourceSet(unsigned index, GPUResourceSet *resources) = 0;

    /** Set the blend state.
     * @param state         Blend state to set. */
    virtual void setBlendState(GPUBlendState *state) = 0;

    /** Set the depth/stencil state.
     * @param state         Depth/stencil state to set. */
    virtual void setDepthStencilState(GPUDepthStencilState *state) = 0;

    /** Set the rasterizer state.
     * @param state         Rasterizer state to set. */
    virtual void setRasterizerState(GPURasterizerState *state) = 0;

    /** Set the viewport.
     * @param viewport      Viewport rectangle in pixels. Must be <= size of
     *                      the current render target. */
    virtual void setViewport(const IntRect &viewport) = 0;

    /** Set the scissor test parameters.
     * @param enable        Whether to enable scissor testing.
     * @param scissor       Scissor rectangle. */
    virtual void setScissor(bool enable, const IntRect &scissor) = 0;

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
     * matching state object at every call. They should only be used within
     * a render pass.
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

    /**
     * Debug methods.
     */

    #ifdef ORION_BUILD_DEBUG

    /** Begin a debug group.
     * @param str           Group string. */
    virtual void beginDebugGroup(const std::string &str) {}

    /** End the current debug group. */
    virtual void endDebugGroup() {}

    #endif
protected:
    GPUManager();

    void destroyStates();

    /** Create a blend state object.
     * @param desc          Descriptor for blend state.
     * @return              Created blend state object. */
    virtual GPUBlendStatePtr createBlendState(const GPUBlendStateDesc &desc);

    /** Create a depth/stencil state object.
     * @param desc          Descriptor for depth/stencil state.
     * @return              Created depth/stencil state object. */
    virtual GPUDepthStencilStatePtr createDepthStencilState(const GPUDepthStencilStateDesc &desc);

    /** Create a rasterizer state object.
     * @param desc          Descriptor for rasterizer state.
     * @return              Created rasterizer state object. */
    virtual GPURasterizerStatePtr createRasterizerState(const GPURasterizerStateDesc &desc);

    /** Create a sampler state object.
     * @param desc          Descriptor for sampler state.
     * @return              Pointer to created sampler state object. */
    virtual GPUSamplerStatePtr createSamplerState(const GPUSamplerStateDesc &desc);
private:
    /** Hash tables of created state objects. */
    HashMap<GPUBlendStateDesc, GPUBlendStatePtr> m_blendStates;
    HashMap<GPUDepthStencilStateDesc, GPUDepthStencilStatePtr> m_depthStencilStates;
    HashMap<GPURasterizerStateDesc, GPURasterizerStatePtr> m_rasterizerStates;
    HashMap<GPUSamplerStateDesc, GPUSamplerStatePtr> m_samplerStates;
};

extern GPUManager *g_gpuManager;

#include "gpu/const_state.h"

#ifdef ORION_BUILD_DEBUG

/** RAII debug group class. */
class GPUDebugGroup {
public:
    /** Begin a debug group.
     * @param str           Group string. */
    explicit GPUDebugGroup(const std::string &str) {
        g_gpuManager->beginDebugGroup(str);
    }

    /** End the debug group. */
    ~GPUDebugGroup() {
        g_gpuManager->endDebugGroup();
    }
};

/** Begin a scoped debug group.
 * @param fmt           Format string and arguments for group name. */
#define GPU_DEBUG_GROUP(fmt...) \
    GPUDebugGroup debugGroup_ ## __LINE__ (String::format(fmt));

/** Begin a debug group.
 * @param fmt           Format string and arguments for group name. */
#define GPU_BEGIN_DEBUG_GROUP(fmt...) \
    g_gpuManager->beginDebugGroup(String::format(fmt));

/** End a debug group. */
#define GPU_END_DEBUG_GROUP() \
    g_gpuManager->endDebugGroup();

#else /* ORION_BUILD_DEBUG */

#define GPU_DEBUG_GROUP(fmt...) do {} while(0)
#define GPU_BEGIN_DEBUG_GROUP(fmt...) do {} while(0)
#define GPU_END_DEBUG_GROUP() do {} while(0)

#endif /* ORION_BUILD_DEBUG */
