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

/**
 * Default object creation methods.
 */

GPUBlendStatePtr GPUManager::createBlendState(const GPUBlendStateDesc &desc) {
    return new GPUBlendState(desc);
}

GPUDepthStencilStatePtr GPUManager::createDepthStencilState(const GPUDepthStencilStateDesc &desc) {
    return new GPUDepthStencilState(desc);
}

GPUIndexDataPtr GPUManager::createIndexData(GPUBuffer *buffer, GPUIndexData::Type type, size_t count, size_t offset) {
    return new GPUIndexData(buffer, type, count, offset);
}

GPUPipelinePtr GPUManager::createPipeline(const GPUPipelineDesc &desc) {
    return new GPUPipeline(desc);
}

GPURasterizerStatePtr GPUManager::createRasterizerState(const GPURasterizerStateDesc &desc) {
    return new GPURasterizerState(desc);
}

GPUSamplerStatePtr GPUManager::createSamplerState(const GPUSamplerStateDesc &desc) {
    return new GPUSamplerState(desc);
}

GPUVertexDataPtr GPUManager::createVertexData(size_t count, GPUVertexInputState *inputState, GPUBufferArray &&buffers) {
    return new GPUVertexData(count, inputState, std::move(buffers));
}

GPUVertexInputStatePtr GPUManager::createVertexInputState(GPUVertexInputStateDesc &&desc) {
    return new GPUVertexInputState(std::move(desc));
}
