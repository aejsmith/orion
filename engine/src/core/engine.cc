/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main class.
 */

#include "core/engine.h"
#include "core/window.h"

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
	m_log = new LogManager;
	orion_log(LogLevel::kInfo, "Orion revision %s built at %s", g_version_string, g_version_timestamp);

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
		orion_abort("Failed to initialize SDL: %s", SDL_GetError());

	/* Create the GPU interface. */
	m_gpu = GPUInterface::create(config);

	/* Create the main window, which properly initializes the GPU interface. */
	m_window = new Window(config, m_gpu);
}

/** Shut down the engine. */
Engine::~Engine() {
	shutdown();
	g_engine = nullptr;
}

/** Shut down the engine. */
void Engine::shutdown() {
	delete m_gpu;
	delete m_window;
	delete m_log;
	SDL_Quit();
}

/**
 * Add a render target to the main rendering loop.
 *
 * Adds an active render target to the main rendering loop to be updated. This
 * should not be called manually - a render target will automatically be added
 * here when a view is added to it.
 *
 * @param target	Render target to add.
 */
void Engine::add_render_target(RenderTarget *target) {
	/* List is sorted by priority. */
	for(auto it = m_render_targets.begin(); it != m_render_targets.end(); ++it) {
		RenderTarget *exist = *it;
		if(target->priority() < exist->priority()) {
			m_render_targets.insert(it, target);
			return;
		}
	}

	/* Insertion point not found, add at end. */
	m_render_targets.push_back(target);
}

/** Remove a render target from the main rendering loop.
 * @param target	Render target to remove. */
void Engine::remove_render_target(RenderTarget *target) {
	m_render_targets.remove(target);
}

bool Engine::loop() {
	/* FIXME: temporary. */
	m_gpu->swap_buffers();

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
