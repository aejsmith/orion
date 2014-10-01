/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GPU interface.
 */

#include "engine/engine.h"

#include "gl/gl.h"

/** Global GPU interface instance. */
EngineGlobal<GPUInterface> g_gpu;

/** Create the GPU interface.
 * @param config	Engine configuration.
 * @return		Pointer to created GPU interface. */
GPUInterface *GPUInterface::create(const EngineConfiguration &config) {
	switch(config.graphicsAPI) {
	case EngineConfiguration::kGLGraphicsAPI:
		return new GLGPUInterface;
	default:
		fatal("Configuration specifies unknown graphics API");
	}
}
