/**
 * @file
 * @copyright           2015 Alex Smith
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

GPUIndexDataPtr GPUManager::createIndexData(GPUBuffer *buffer, GPUIndexData::Type type, size_t count) {
    return new GPUIndexData(buffer, type, count);
}

GPUPipelinePtr GPUManager::createPipeline(const GPUShaderArray &shaders) {
    return new GPUPipeline(shaders);
}

GPUVertexDataPtr GPUManager::createVertexData(size_t count, GPUVertexFormat *format, GPUBufferArray &buffers) {
    return new GPUVertexData(count, format, buffers);
}

GPUVertexFormatPtr GPUManager::createVertexFormat(VertexBufferLayoutArray &buffers, VertexAttributeArray &attributes) {
    return new GPUVertexFormat(buffers, attributes);
}
