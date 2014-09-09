/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main window class.
 */

#pragma once

#include "engine/render_target.h"

class GPUInterface;

struct EngineConfiguration;
struct SDL_Window;

/** Engine main window class. */
class Window : public RenderTarget {
public:
	Window(const EngineConfiguration &config, GPUInterface *gpu);
	~Window();

	void setTitle(const std::string &title);

	/** @return		SDL window. */
	SDL_Window *sdlWindow() const { return m_sdlWindow; }
	/** @return		Size of the window (in pixels). */
	glm::ivec2 size() const override { return m_size; }
private:
	SDL_Window *m_sdlWindow;	/**< SDL window. */
	glm::ivec2 m_size;		/**< Size of the window. */
};

extern EngineGlobal<Window> g_mainWindow;
