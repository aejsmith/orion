/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main class.
 */

#include "core/engine.h"
#include "core/window.h"

#include "gpu/gpu.h"

#include "world/camera.h"
#include "world/world.h"

#include <SDL.h>

/** Global instance of the engine. */
Engine *g_engine = nullptr;

/** Initialize the engine.
 * @param config	Engine configuration structure. */
Engine::Engine(const EngineConfiguration &config) :
	m_config(config),
	m_world(nullptr)
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
	if(m_world)
		delete m_world;

	delete m_gpu;
	delete m_window;
	delete m_log;
	SDL_Quit();
}

/** Run the engine main loop. */
void Engine::run() {
	uint32_t last_tick = 0;
	uint32_t last_fps = 0;
	uint32_t frames = 0;

	while(true) {
		/* Handle SDL events. */
		SDL_Event event;
		if(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYUP:
				if(event.key.keysym.sym == SDLK_ESCAPE)
					return;

				break;
			case SDL_QUIT:
				return;
			}
		}

		uint32_t tick = SDL_GetTicks();

		if(last_tick && tick != last_tick) {
			/* Update the world. */
			float dt = static_cast<float>(tick - last_tick) / 1000.0f;
			m_world->tick(dt);
		}

		last_tick = tick;

		/* Update FPS counter. */
		if(!last_fps || (tick - last_fps) > 1000) {
			unsigned fps = (last_fps)
				? static_cast<float>(frames) / (static_cast<float>(tick - last_fps) / 1000.0f)
				: 0;

			m_window->set_title(m_config.title + " [FPS: " + std::to_string(fps) + "]");
			last_fps = tick;
			frames = 0;
		}

		/* Update all render targets. */
		for(RenderTarget *target : m_render_targets) {
			// FIXME: Where does this go? Should clear from camera with
			// world background colour, but then we need a rect constraint
			// to clear to only clear viewport.
			m_gpu->clear(
				RenderBuffer::kColourBuffer | RenderBuffer::kDepthBuffer | RenderBuffer::kStencilBuffer,
				glm::vec4(0.0, 0.0, 0.0, 1.0), 1.0, 0);

			for(CameraComponent *camera : target->cameras())
				camera->render();
		}

		m_gpu->end_frame(m_config.display_vsync);
		frames++;
	}
}

/** Create a world and make it the active world.
 * @return		Created world. */
World *Engine::create_world() {
	if(m_world)
		delete m_world;

	m_world = new World;
	return m_world;
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
