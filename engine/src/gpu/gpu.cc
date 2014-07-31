/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GPU interface.
 */

#include "gl/gpu.h"

/** Create the GPU interface.
 * @param config	Engine configuration.
 * @return		Pointer to created GPU interface. */
GPUInterface *GPUInterface::create(const EngineConfiguration &config) {
	switch(config.graphics_api) {
	case EngineConfiguration::kGLGraphicsAPI:
		return new GLGPUInterface;
	default:
		orion_abort("Configuration specifies unknown graphics API");
	}
}

/**
 * Default object creation methods.
 */

/** Create a vertex format descriptor.
 * @return		Pointer to created vertex format descriptor. */
VertexFormatPtr GPUInterface::create_vertex_format() {
	VertexFormat *format = new VertexFormat;
	return VertexFormatPtr(format);
}

/** Create a vertex data object.
 * @return		Pointer to created vertex data object. */
VertexDataPtr GPUInterface::create_vertex_data(size_t vertices) {
	VertexData *data = new VertexData(vertices);
	return VertexDataPtr(data);
}

/** Create an index data object.
 * @param buffer	Buffer holding the index data.
 * @param type		Type of index elements.
 * @param count		Number of indices.
 * @return		Pointer to created index data object. */
IndexDataPtr GPUInterface::create_index_data(const GPUBufferPtr &buffer, IndexData::Type type, size_t count) {
	IndexData *data = new IndexData(buffer, type, count);
	return IndexDataPtr(data);
}

/** Create a pipeline object.
 * @return		Pointer to created pipeline. */
GPUPipelinePtr GPUInterface::create_pipeline() {
	GPUPipeline *pipeline = new GPUPipeline;
	return GPUPipelinePtr(pipeline);
}
