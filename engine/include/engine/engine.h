/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main class.
 */

#pragma once

#include "core/core.h"

#include <list>

class Game;
class RenderTarget;
class World;

/** Engine configuration.
 * @todo		Eventually this will only contain static configuration
 *			for the application, e.g. the title. Dynamic settings
 *			like screen resolution will move to some preferences
 *			class (a key/value database) that will save and restore
 *			settings. */
struct EngineConfiguration {
	/** Graphics API IDs. */
	enum GraphicsAPI {
		kGLGraphicsAPI,
	};
public:
	/** Title of the game. */
	std::string title;

	/** Graphics API to use. */
	GraphicsAPI graphicsAPI;

	/** Display parameters. */
	uint32_t displayWidth;		/**< Screen width. */
	uint32_t displayHeight;		/**< Screen height. */
	bool displayFullscreen;		/**< Whether the window should be fullscreen. */
	bool displayVsync;		/**< Whether to synchronize updates with vertical retrace. */
};

/** Main class of the engine. */
class Engine : Noncopyable {
public:
	Engine(const EngineConfiguration &config);
	~Engine();

	void run();

	/** @return		Engine configuration. */
	const EngineConfiguration &config() const { return m_config; }
	/** @return		Game instance. */
	Game *game() const { return m_game; }

	/**
	 * World management.
	 */

	World *createWorld();

	/** @return		Active game world. */
	World *world() const { return m_world; }

	/**
	 * Rendering loop.
	 */

	void addRenderTarget(RenderTarget *target);
	void removeRenderTarget(RenderTarget *target);
private:
	/**
	 * Main loop functions.
	 */

	bool pollEvents();
	void tick();
	void renderAllTargets();
private:
	EngineConfiguration m_config;	/**< Engine configuration. */
	Game *m_game;			/**< Game instance. */
	World *m_world;			/**< Active game world. */

	/** List of render targets. */
	std::list<RenderTarget *> m_renderTargets;

	/** Timing information. */
	uint32_t m_lastTick;		/**< Last tick time. */
	uint32_t m_lastFPS;		/**< Last FPS value. */
	uint32_t m_frames;		/**< Number of frames rendered since last FPS update. */
};

extern Engine *g_engine;
