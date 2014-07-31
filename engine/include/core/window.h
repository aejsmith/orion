/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main window class.
 */

#ifndef ORION_CORE_WINDOW_H
#define ORION_CORE_WINDOW_H

#include "core/engine.h"

#include "render/render_target.h"

class GPUInterface;
struct SDL_Window;

/** Engine main window class. */
class Window : public RenderTarget {
public:
	Window(const EngineConfiguration &config, GPUInterface *gpu);
	~Window();

	/** @return		Size of the window (in pixels). */
	glm::ivec2 size() const { return m_size; }
private:
	SDL_Window *m_window;		/**< SDL window. */
	glm::ivec2 m_size;		/**< Size of the window. */
};

#endif /* ORION_CORE_WINDOW_H */
