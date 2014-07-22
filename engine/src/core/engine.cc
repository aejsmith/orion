/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main class.
 */

#include "core/engine.h"

#include "gpu/gpu.h"

#include <SDL.h>

/** Global instance of the engine. */
Engine *g_engine = nullptr;

/** Initialize the engine.
 * @param config	Engine configuration structure. */
Engine::Engine(const EngineConfiguration &config) :
	m_config(config)
{
	orion_assert(!g_engine);
	g_engine = this;

	/* Initialize the log. */
	new LogManager();
	orion_log(LogLevel::kInfo, "Orion revision %s built at %s", g_version_string, g_version_timestamp);

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
		orion_abort("Failed to initialize SDL: %s", SDL_GetError());

	/* Set up the graphics API. */
	GPUInterface::init(m_config);
}

/** Shut down the engine. */
Engine::~Engine() {
	shutdown();
	g_engine = nullptr;
}

bool Engine::loop() {
	/* FIXME: temporary. */
	g_gpu->swap_buffers();

	/* Handle SDL events. */
	SDL_Event event;
	if(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_KEYUP:
			if(event.key.keysym.sym == SDLK_ESCAPE)
				return false;
			break;
		case SDL_QUIT:
			return false;
		}
	}

	return true;
}

/** Shut down the engine. */
void Engine::shutdown() {
	delete g_log_manager;
	SDL_Quit();
}
