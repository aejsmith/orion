/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main window class.
 */

#include "engine/window.h"

#include "gpu/gpu.h"

#include <SDL.h>

/** Create the main window.
 * @param config	Engine configuration structure.
 * @param gpu		GPU interface. */
Window::Window(const EngineConfiguration &config, GPUInterface *gpu) :
	RenderTarget(kWindowPriority),
	m_size(config.displayWidth, config.displayHeight)
{
	uint32_t flags = 0;

	if(config.displayFullscreen)
		flags |= SDL_WINDOW_FULLSCREEN;
	if(config.graphicsAPI == EngineConfiguration::kGLGraphicsAPI)
		flags |= SDL_WINDOW_OPENGL;

	m_window = SDL_CreateWindow(config.title.c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		config.displayWidth, config.displayHeight,
		flags);
	if(!m_window)
		orionAbort("Failed to create main window: %s", SDL_GetError());

	/* Initialize the GPU interface properly. */
	gpu->init(m_window);
}

/** Destroy the window. */
Window::~Window() {
	SDL_DestroyWindow(m_window);
}

/** Set the window title.
 * @param title		Title of the window. */
void Window::setTitle(const std::string &title) {
	SDL_SetWindowTitle(m_window, title.c_str());
}
