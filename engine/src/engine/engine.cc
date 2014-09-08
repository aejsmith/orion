/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main class.
 */

#include "core/filesystem.h"

#include "engine/asset_manager.h"
#include "engine/engine.h"
#include "engine/window.h"

#include "gpu/gpu.h"

#include "world/camera_component.h"
#include "world/world.h"

#include <SDL.h>

/** Global instance of the engine. */
Engine *g_engine = nullptr;

/** Initialize the engine.
 * @param config	Engine configuration structure. */
Engine::Engine(const EngineConfiguration &config) :
	m_config(config),
	m_world(nullptr),
	m_lastTick(0),
	m_lastFPS(0),
	m_frames(0)
{
	orionAssert(!g_engine);
	g_engine = this;

	/* Initialize the log. */
	m_log = new LogManager;
	orionLog(LogLevel::kInfo, "Orion revision %s built at %s", g_versionString, g_versionTimestamp);

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
		orionAbort("Failed to initialize SDL: %s", SDL_GetError());

	/* Initialize platform systems. */
	m_filesystem = platform::createFilesystem();

	/* Create the GPU interface, then create the main window which properly
	 * initializes the GPU interface. */
	m_gpu = GPUInterface::create(config);
	m_window = new Window(config, m_gpu);

	/* Initialize other global systems. */
	m_assets = new AssetManager;
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

	delete m_assets;
	delete m_gpu;
	delete m_window;
	delete m_filesystem;
	delete m_log;

	SDL_Quit();
}

/** Run the engine main loop. */
void Engine::run() {
	while(true) {
		if(!pollEvents())
			return;

		tick();
		renderAllTargets();

		/* Present the final rendered frame. */
		m_gpu->endFrame(m_config.displayVsync);
		m_frames++;
	}
}

/** Poll for pending SDL events.
 * @return		Whether to continue executing. */
bool Engine::pollEvents() {
	SDL_Event event;

	while(SDL_PollEvent(&event)) {
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

/** Tick the world. */
void Engine::tick() {
	uint32_t tick = SDL_GetTicks();

	if(m_lastTick && tick != m_lastTick) {
		/* Update the world. */
		float dt = static_cast<float>(tick - m_lastTick) / 1000.0f;
		m_world->tick(dt);
	}

	m_lastTick = tick;

	/* Update FPS counter. */
	if(!m_lastFPS || (tick - m_lastFPS) > 1000) {
		unsigned fps = (m_lastFPS)
			? static_cast<float>(m_frames) / (static_cast<float>(tick - m_lastFPS) / 1000.0f)
			: 0;

		m_window->setTitle(m_config.title + " [FPS: " + std::to_string(fps) + "]");
		m_lastFPS = tick;
		m_frames = 0;
	}
}

/** Render all render targets. */
void Engine::renderAllTargets() {
	for(RenderTarget *target : m_renderTargets) {
		// FIXME: Where does this go? Clear settings should go in
		// SceneView, need a rect constraint to clear to only clear
		// viewport.
		m_gpu->clear(
			RenderBuffer::kColourBuffer | RenderBuffer::kDepthBuffer | RenderBuffer::kStencilBuffer,
			glm::vec4(0.0, 0.0, 0.0, 1.0), 1.0, 0);

		for(CameraComponent *camera : target->cameras())
			camera->render();
	}
}

/** Create a world and make it the active world.
 * @return		Created world. */
World *Engine::createWorld() {
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
void Engine::addRenderTarget(RenderTarget *target) {
	/* List is sorted by priority. */
	for(auto it = m_renderTargets.begin(); it != m_renderTargets.end(); ++it) {
		RenderTarget *exist = *it;
		if(target->priority() < exist->priority()) {
			m_renderTargets.insert(it, target);
			return;
		}
	}

	/* Insertion point not found, add at end. */
	m_renderTargets.push_back(target);
}

/** Remove a render target from the main rendering loop.
 * @param target	Render target to remove. */
void Engine::removeRenderTarget(RenderTarget *target) {
	m_renderTargets.remove(target);
}