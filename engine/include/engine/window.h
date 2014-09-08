/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main window class.
 */

#pragma once

#include "engine/engine.h"
#include "engine/render_target.h"

#include <string>

class GPUInterface;
struct SDL_Window;

/** Engine main window class. */
class Window : public RenderTarget {
public:
	Window(const EngineConfiguration &config, GPUInterface *gpu);
	~Window();

	void setTitle(const std::string &title);

	/** @return		SDL window. */
	SDL_Window *sdl() const { return m_window; }
	/** @return		Size of the window (in pixels). */
	glm::ivec2 size() const override { return m_size; }
private:
	SDL_Window *m_window;		/**< SDL window. */
	glm::ivec2 m_size;		/**< Size of the window. */
};
