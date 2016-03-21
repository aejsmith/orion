/*
 * Copyright (C) 2015 Alex Smith
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

#include "engine/engine.h"

#include "gl/gl.h"

/** Global GPU manager instance. */
EngineGlobal<GPUManager> g_gpuManager;

/** Create the GPU manager.
 * @param config        Engine configuration.
 * @return              Pointer to created GPU manager. */
GPUManager *GPUManager::create(const EngineConfiguration &config) {
    switch (config.graphicsAPI) {
        case EngineConfiguration::kGLGraphicsAPI:
            return new GLGPUManager;
        default:
            fatal("Configuration specifies unknown graphics API");
    }
}

/**
 * Default object creation methods.
 */

GPUIndexDataPtr GPUManager::createIndexData(GPUBuffer *buffer, GPUIndexData::Type type, size_t count, size_t offset) {
    return new GPUIndexData(buffer, type, count, offset);
}

GPUPipelinePtr GPUManager::createPipeline(const GPUPipelineDesc &desc) {
    return new GPUPipeline(desc);
}

GPUVertexDataPtr GPUManager::createVertexData(size_t count, GPUVertexFormat *format, GPUBufferArray &buffers) {
    return new GPUVertexData(count, format, buffers);
}

GPUVertexFormatPtr GPUManager::createVertexFormat(VertexBufferLayoutArray &buffers, VertexAttributeArray &attributes) {
    return new GPUVertexFormat(buffers, attributes);
}
