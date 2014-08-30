/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GPU interface.
 */

#include "gl/gl.h"

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
