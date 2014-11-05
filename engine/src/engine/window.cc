/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine main window class.
 */

#include "engine/engine.h"
#include "engine/window.h"

#include "gpu/gpu.h"

#include <SDL.h>

/** Engine main window. */
EngineGlobal<Window> g_mainWindow;

/** Create the main window.
 * @param config	Engine configuration structure. */
Window::Window(const EngineConfiguration &config) :
	RenderTarget(kWindowPriority),
	m_width(config.displayWidth),
	m_height(config.displayHeight)
{
	uint32_t flags = 0;

	if(config.displayFullscreen)
		flags |= SDL_WINDOW_FULLSCREEN;
	if(config.graphicsAPI == EngineConfiguration::kGLGraphicsAPI)
		flags |= SDL_WINDOW_OPENGL;

	m_sdlWindow = SDL_CreateWindow(
		config.title.c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		config.displayWidth, config.displayHeight,
		flags);
	if(!m_sdlWindow)
		fatal("Failed to create main window: %s", SDL_GetError());
}

/** Destroy the window. */
Window::~Window() {
	SDL_DestroyWindow(m_sdlWindow);
}

/** Set the window as the render target. */
void Window::set() {
	g_gpu->setRenderTarget(nullptr);
}

/** Set the window title.
 * @param title		Title of the window. */
void Window::setTitle(const std::string &title) {
	SDL_SetWindowTitle(m_sdlWindow, title.c_str());
}
