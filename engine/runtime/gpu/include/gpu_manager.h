/*
 * Copyright (C) 2015-2017 Alex Smith
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
#include "gpu/command_list.h"
#include "gpu/index_data.h"
#include "gpu/pipeline.h"
#include "gpu/program.h"
#include "gpu/query_pool.h"
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

    virtual GPUIndexDataPtr createIndexData(GPUIndexDataDesc &&desc);

    /** Create a pipeline object.
     * @param desc          Parameters for the pipeline.
     * @return              Pointer to created pipeline. */
    virtual GPUPipelinePtr createPipeline(GPUPipelineDesc &&desc) = 0;

    /** Create a query pool.
     * @param desc          Descriptor for the query pool.
     * @return              Pointer to created pool. */
    virtual GPUQueryPoolPtr createQueryPool(const GPUQueryPoolDesc &desc) = 0;

    virtual GPURenderPassPtr createRenderPass(GPURenderPassDesc &&desc);

    /** Create a texture.
     * @param desc          Descriptor containing texture parameters.
     * @return              Pointer to created texture. */
    virtual GPUTexturePtr createTexture(const GPUTextureDesc &desc) = 0;

    /** Create a texture view.
     * @param desc          Descriptor containing view parameters.
     * @return              Pointer to created texture view. */
    virtual GPUTexturePtr createTextureView(const GPUTextureViewDesc &desc) = 0;

    virtual GPUVertexDataPtr createVertexData(GPUVertexDataDesc &&desc);

    /**
     * State object methods.
     */

    GPUBlendStatePtr getBlendState(const GPUBlendStateDesc &desc = GPUBlendStateDesc());
    GPUDepthStencilStatePtr getDepthStencilState(const GPUDepthStencilStateDesc &desc = GPUDepthStencilStateDesc());
    GPURasterizerStatePtr getRasterizerState(const GPURasterizerStateDesc &desc = GPURasterizerStateDesc());
    GPUSamplerStatePtr getSamplerState(const GPUSamplerStateDesc &desc = GPUSamplerStateDesc());
    GPUVertexDataLayoutPtr getVertexDataLayout(const GPUVertexDataLayoutDesc &desc);

    /**
     * Shader methods.
     */

    virtual GPUResourceSetLayoutPtr createResourceSetLayout(GPUResourceSetLayoutDesc &&desc);
    virtual GPUResourceSetPtr createResourceSet(GPUResourceSetLayout *layout);

    /** Create a GPU program from a SPIR-V binary.
     * @param desc          Descriptor for the program.
     * @return              Pointer to created program. */
    virtual GPUProgramPtr createProgram(GPUProgramDesc &&desc) = 0;

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
    virtual void blit(const GPUTextureImageRef &source,
                      const GPUTextureImageRef &dest,
                      glm::ivec2 sourcePos,
                      glm::ivec2 destPos,
                      glm::ivec2 size) = 0;

    /**
     * Render pass methods.
     */

    /**
     * Begin a render pass.
     *
     * Begins a new render pass instance. The render pass defines the targets
     * that will be drawn to. This will return a command list that can be used
     * to record the commands for the render pass. Once all commands have been
     * recorded, it can be submitted with submitRenderPass().
     *
     * The command list returned will have the following default state set:
     *
     *  - Blend, depth/stencil and rasterizer states will be set to the default
     *    states.
     *  - Viewport will be set to the specified render area.
     *  - Scissor test will be disabled.
     *
     * Note that it is possible to record multiple render passes in parallel
     * by calling this function multiple times to get command lists for each,
     * as no work is actually done until submitRenderPass(); this function just
     * does preliminary setup and creates a command list.
     *
     * @param desc          Descriptor for the render pass instance.
     *
     * @return              Command list to record pass into.
     */
    virtual GPUCommandList *beginRenderPass(const GPURenderPassInstanceDesc &desc) = 0;

    /** Submit a render pass.
     * @param cmdList       Command list for the pass (will be deleted and must
     *                      not be used after this call). */
    virtual void submitRenderPass(GPUCommandList *cmdList) = 0;

    /**
     * Query methods.
     */

    /** Begin a query.
     * @param queryPool     Query pool the query is in.
     * @param index         Index of the query to begin. */
    //virtual void beginQuery(GPUQueryPool *queryPool, uint32_t index) = 0;

    /** End a query.
     * @param queryPool     Query pool the query is in.
     * @param index         Index of the query to end. */
    virtual void endQuery(GPUQueryPool *queryPool, uint32_t index) = 0;

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

    /** Create a vertex data layout object.
     * @param desc          Descriptor for vertex data layout.
     * @return              Pointer to created vertex data layout object. */
    virtual GPUVertexDataLayoutPtr createVertexDataLayout(const GPUVertexDataLayoutDesc &desc);
private:
    /** Hash tables of created state objects. */
    HashMap<GPUBlendStateDesc, GPUBlendStatePtr> m_blendStates;
    HashMap<GPUDepthStencilStateDesc, GPUDepthStencilStatePtr> m_depthStencilStates;
    HashMap<GPURasterizerStateDesc, GPURasterizerStatePtr> m_rasterizerStates;
    HashMap<GPUSamplerStateDesc, GPUSamplerStatePtr> m_samplerStates;
    HashMap<GPUVertexDataLayoutDesc, GPUVertexDataLayoutPtr> m_vertexDataLayouts;
};

extern GPUManager *g_gpuManager;

#ifdef ORION_BUILD_DEBUG

/** RAII debug group class. */
class GPUDebugGroup {
public:
    /** Begin a debug group.
     * @param cmdList       GPU command list.
     * @param str           Group string. */
    GPUDebugGroup(GPUCommandList *cmdList, const std::string &str) :
        m_cmdList(cmdList)
    {
        if (cmdList) {
            m_cmdList->beginDebugGroup(str);
        } else {
            g_gpuManager->beginDebugGroup(str);
        }
    }

    /** End the debug group. */
    ~GPUDebugGroup() {
        if (m_cmdList) {
            m_cmdList->endDebugGroup();
        } else {
            g_gpuManager->endDebugGroup();
        }
    }
private:
    GPUCommandList *m_cmdList;
};

/** Begin a scoped debug group.
 * @param fmt           Format string and arguments for group name. */
#define GPU_DEBUG_GROUP(...) \
    GPUDebugGroup debugGroup_ ## __LINE__ (nullptr, String::format(__VA_ARGS__));

/** Begin a debug group.
 * @param cmdList       GPU command list.
 * @param fmt           Format string and arguments for group name. */
#define GPU_BEGIN_DEBUG_GROUP(...) \
    g_gpuManager->beginDebugGroup(String::format(__VA_ARGS__));

/** End a debug group. */
#define GPU_END_DEBUG_GROUP() \
    g_gpuManager->endDebugGroup();

/** Begin a scoped debug group on a command list.
 * @param cmdList       GPU command list.
 * @param fmt           Format string and arguments for group name. */
#define GPU_CMD_DEBUG_GROUP(cmdList, ...) \
    GPUDebugGroup debugGroup_ ## __LINE__ ((cmdList), String::format(__VA_ARGS__));

/** Begin a debug group on a command list.
 * @param cmdList       GPU command list.
 * @param fmt           Format string and arguments for group name. */
#define GPU_CMD_BEGIN_DEBUG_GROUP(cmdList, ...) \
    (cmdList)->beginDebugGroup(String::format(__VA_ARGS__));

/** End a debug group on a command list.
 * @param cmdList       GPU command list. */
#define GPU_CMD_END_DEBUG_GROUP(cmdList) \
    (cmdList)->endDebugGroup();

#else /* ORION_BUILD_DEBUG */

#define GPU_DEBUG_GROUP(...) do {} while(0)
#define GPU_BEGIN_DEBUG_GROUP(...) do {} while(0)
#define GPU_END_DEBUG_GROUP() do {} while(0)

#define GPU_CMD_DEBUG_GROUP(cmdList, ...) do {} while(0)
#define GPU_CMD_BEGIN_DEBUG_GROUP(cmdList, ...) do {} while(0)
#define GPU_CMD_END_DEBUG_GROUP(cmdList) do {} while(0)

#endif /* ORION_BUILD_DEBUG */
