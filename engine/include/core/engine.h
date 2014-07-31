/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main class/header.
 */

#ifndef ORION_CORE_ENGINE_H
#define ORION_CORE_ENGINE_H

#include "core/defs.h"

#include "lib/error.h"
#include "lib/log.h"
#include "lib/noncopyable.h"
#include "lib/version.h"

#include <list>
#include <string>

class GPUInterface;
class LogManager;
class RenderTarget;
class Window;

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
	GraphicsAPI graphics_api;

	/** Display parameters. */
	uint32_t display_width;		/**< Screen width. */
	uint32_t display_height;	/**< Screen height. */
	bool display_fullscreen;	/**< Whether the window should be fullscreen. */
	bool display_vsync;		/**< Whether to synchronize updates with vertical retrace. */
};

/** Main class of the engine. */
class Engine : Noncopyable {
public:
	Engine(const EngineConfiguration &config);
	~Engine();

	bool loop();
	void shutdown();

	/** @return		Engine configuration. */
	const EngineConfiguration &config() const { return m_config; }

	/**
	 * Global resources.
	 */

	/** @return		Log manager. */
	LogManager *log() const { return m_log; }
	/** @return		GPU interface. */
	GPUInterface *gpu() const { return m_gpu; }
	/** @return		Engine main window. */
	Window *window() const { return m_window; }

	/**
	 * Rendering loop.
	 */

	void add_render_target(RenderTarget *target);
	void remove_render_target(RenderTarget *target);
private:
	/** Type of a list of render targets. */
	typedef std::list<RenderTarget *> RenderTargetList;
private:
	/** Engine configuration. */
	EngineConfiguration m_config;

	/** Global resources. */
	LogManager *m_log;		/**< Log manager. */
	GPUInterface *m_gpu;		/**< GPU interface. */
	Window *m_window;		/**< Main window. */

	/** List of render targets. */
	RenderTargetList m_render_targets;
};

extern Engine *g_engine;

#endif /* ORION_CORE_ENGINE_H */
