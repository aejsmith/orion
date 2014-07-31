/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main window class.
 */

#include "core/window.h"

#include "gpu/gpu.h"

#include <SDL.h>

/** Create the main window.
 * @param config	Engine configuration structure.
 * @param gpu		GPU interface. */
Window::Window(const EngineConfiguration &config, GPUInterface *gpu) :
	RenderTarget(kWindowPriority),
	m_size(config.display_width, config.display_height)
{
	uint32_t flags = 0;

	if(config.display_fullscreen)
		flags |= SDL_WINDOW_FULLSCREEN;
	if(config.graphics_api == EngineConfiguration::kGLGraphicsAPI)
		flags |= SDL_WINDOW_OPENGL;

	m_window = SDL_CreateWindow(config.title.c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		config.display_width, config.display_height,
		flags);
	if(!m_window)
		orion_abort("Failed to create main window: %s", SDL_GetError());

	/* Initialize the GPU interface properly. */
	gpu->init(m_window);
}

/** Destroy the window. */
Window::~Window() {
	SDL_DestroyWindow(m_window);
}
