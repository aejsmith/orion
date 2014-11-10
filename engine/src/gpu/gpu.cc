/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               GPU interface.
 */

#include "engine/engine.h"

#include "gl/gl.h"

/** Global GPU interface instance. */
EngineGlobal<GPUInterface> g_gpu;

/** Create the GPU interface.
 * @param config        Engine configuration.
 * @return              Pointer to created GPU interface. */
GPUInterface *GPUInterface::create(const EngineConfiguration &config) {
    switch (config.graphicsAPI) {
        case EngineConfiguration::kGLGraphicsAPI:
            return new GLGPUInterface;
        default:
            fatal("Configuration specifies unknown graphics API");
    }
}

/**
 * Default object creation methods.
 */

GPUIndexDataPtr GPUInterface::createIndexData(GPUBuffer *buffer, GPUIndexData::Type type, size_t count) {
    return new GPUIndexData(buffer, type, count);
}

GPUPipelinePtr GPUInterface::createPipeline(const GPUShaderArray &shaders) {
    return new GPUPipeline(shaders);
}

GPUVertexDataPtr GPUInterface::createVertexData(size_t count, GPUVertexFormat *format, GPUBufferArray &buffers) {
    return new GPUVertexData(count, format, buffers);
}

GPUVertexFormatPtr GPUInterface::createVertexFormat(VertexBufferLayoutArray &buffers, VertexAttributeArray &attributes) {
    return new GPUVertexFormat(buffers, attributes);
}
