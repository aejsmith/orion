/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main class/header.
 */

#pragma once

#include "core/core.h"

#include <list>
#include <string>

class AssetManager;
class Filesystem;
class GPUInterface;
class LogManager;
class RenderTarget;
class Window;
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

	void shutdown();
	void run();

	/** @return		Engine configuration. */
	const EngineConfiguration &config() const { return m_config; }

	/**
	 * Global resources.
	 */

	/** @return		Asset manager. */
	AssetManager *assets() const { return m_assets; }
	/** @return		Filesystem interface. */
	Filesystem *filesystem() const { return m_filesystem; }
	/** @return		GPU interface. */
	GPUInterface *gpu() const { return m_gpu; }
	/** @return		Log manager. */
	LogManager *log() const { return m_log; }
	/** @return		Engine main window. */
	Window *window() const { return m_window; }

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
	/** Engine configuration. */
	EngineConfiguration m_config;

	/** Global resources. */
	AssetManager *m_assets;		/**< Asset manager. */
	Filesystem *m_filesystem;	/**< Filesystem interface. */
	GPUInterface *m_gpu;		/**< GPU interface. */
	LogManager *m_log;		/**< Log manager. */
	Window *m_window;		/**< Main window. */

	/** Active game world. */
	World *m_world;

	/** List of render targets. */
	std::list<RenderTarget *> m_renderTargets;

	/** Timing information. */
	uint32_t m_lastTick;		/**< Last tick time. */
	uint32_t m_lastFPS;		/**< Last FPS value. */
	uint32_t m_frames;		/**< Number of frames rendered since last FPS update. */
};

extern Engine *g_engine;
