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
 * @brief               GPU manager.
 */

#include "gpu/gpu_manager.h"

/** Global GPU manager instance. */
GPUManager *g_gpuManager;

/** Initialise the GPU manager. */
GPUManager::GPUManager() {}

/** Destroy the GPU manager. */
GPUManager::~GPUManager() {}

/** Get a (potentially pre-existing) blend state object.
 * @param desc          Descriptor for blend state.
 * @return              Created blend state object. */
GPUBlendStatePtr GPUManager::getBlendState(const GPUBlendStateDesc &desc) {
    auto exist = m_blendStates.find(desc);
    if (exist != m_blendStates.end())
        return exist->second;

    GPUBlendStatePtr ret = createBlendState(desc);
    m_blendStates.emplace(std::make_pair(desc, ret));
    return ret;
}

/** Get a (potentially pre-existing) depth/stencil state object.
 * @param desc          Descriptor for depth/stencil state.
 * @return              Created depth/stencil state object. */
GPUDepthStencilStatePtr GPUManager::getDepthStencilState(const GPUDepthStencilStateDesc &desc) {
    auto exist = m_depthStencilStates.find(desc);
    if (exist != m_depthStencilStates.end())
        return exist->second;

    GPUDepthStencilStatePtr ret = createDepthStencilState(desc);
    m_depthStencilStates.emplace(std::make_pair(desc, ret));
    return ret;
}

/** Get a (potentially pre-existing) rasterizer state object.
 * @param desc          Descriptor for rasterizer state.
 * @return              Created rasterizer state object. */
GPURasterizerStatePtr GPUManager::getRasterizerState(const GPURasterizerStateDesc &desc) {
    auto exist = m_rasterizerStates.find(desc);
    if (exist != m_rasterizerStates.end())
        return exist->second;

    GPURasterizerStatePtr ret = createRasterizerState(desc);
    m_rasterizerStates.emplace(std::make_pair(desc, ret));
    return ret;
}

/** Get a (potentially pre-existing) sampler state object.
 * @param desc          Descriptor for sampler state.
 * @return              Pointer to created sampler state object. */
GPUSamplerStatePtr GPUManager::getSamplerState(const GPUSamplerStateDesc &desc) {
    auto exist = m_samplerStates.find(desc);
    if (exist != m_samplerStates.end())
        return exist->second;

    GPUSamplerStatePtr ret = createSamplerState(desc);
    m_samplerStates.emplace(std::make_pair(desc, ret));
    return ret;
}

/**
 * Default object creation methods.
 */

/** Create an index data object.
 * @param buffer        Buffer holding the index data.
 * @param type          Type of index elements.
 * @param count         Number of indices.
 * @param offset        Offset of the indices in the buffer.
 * @return              Pointer to created index data object. */
GPUIndexDataPtr GPUManager::createIndexData(
    GPUBuffer *buffer,
    GPUIndexData::Type type,
    size_t count,
    size_t offset)
{
    return new GPUIndexData(buffer, type, count, offset);
}

/** Create a vertex data object.
 * @param count         Total number of vertices.
 * @param inputState    Vertex input state.
 * @param buffers       Array of buffers for each binding in the input state.
 * @return              Pointer to created vertex data object. */
GPUVertexDataPtr GPUManager::createVertexData(
    size_t count,
    GPUVertexInputState *inputState,
    GPUBufferArray &&buffers)
{
    return new GPUVertexData(count, inputState, std::move(buffers));
}

/** Create a blend state object.
 * @param desc          Descriptor for blend state.
 * @return              Created blend state object. */
GPUBlendStatePtr GPUManager::createBlendState(const GPUBlendStateDesc &desc) {
    return new GPUBlendState(desc);
}

/** Create a depth/stencil state object.
 * @param desc          Descriptor for depth/stencil state.
 * @return              Created depth/stencil state object. */
GPUDepthStencilStatePtr GPUManager::createDepthStencilState(const GPUDepthStencilStateDesc &desc) {
    return new GPUDepthStencilState(desc);
}

/** Create a rasterizer state object.
 * @param desc          Descriptor for rasterizer state.
 * @return              Created rasterizer state object. */
GPURasterizerStatePtr GPUManager::createRasterizerState(const GPURasterizerStateDesc &desc) {
    return new GPURasterizerState(desc);
}

/** Create a sampler state object.
 * @param desc          Descriptor for sampler state.
 * @return              Pointer to created sampler state object. */
GPUSamplerStatePtr GPUManager::createSamplerState(const GPUSamplerStateDesc &desc) {
    return new GPUSamplerState(desc);
}

/** Create a vertex input state object.
 * @param desc          Descriptor for vertex input state.
 * @return              Pointer to created vertex input state object. */
GPUVertexInputStatePtr GPUManager::createVertexInputState(GPUVertexInputStateDesc &&desc) {
    return new GPUVertexInputState(std::move(desc));
}
