/*
 * Copyright (C) 2015 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               Engine main window class.
 */

#include "engine/engine.h"
#include "engine/window.h"

#include "gpu/gpu_manager.h"

#include <SDL.h>

/** Engine main window. */
EngineGlobal<Window> g_mainWindow;

/** Create the main window.
 * @param config        Engine configuration structure. */
Window::Window(const EngineConfiguration &config) :
    RenderTarget(kWindowPriority),
    m_width(config.displayWidth),
    m_height(config.displayHeight)
{
    uint32_t flags = 0;

    if (config.displayFullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;
    if (config.graphicsAPI == EngineConfiguration::kGLGraphicsAPI)
        flags |= SDL_WINDOW_OPENGL;

    m_sdlWindow = SDL_CreateWindow(
        config.title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        config.displayWidth, config.displayHeight,
        flags);
    if (!m_sdlWindow)
        fatal("Failed to create main window: %s", SDL_GetError());
}

/** Destroy the window. */
Window::~Window() {
    SDL_DestroyWindow(m_sdlWindow);
}

/** Get the target GPU texture image reference for this render target.
 * @param ref           Image reference structure to fill in. */
void Window::gpu(GPUTextureImageRef &ref) {
    ref.texture = nullptr;
    ref.layer = 0;
}

/** Set the window title.
 * @param title         Title of the window. */
void Window::setTitle(const std::string &title) {
    SDL_SetWindowTitle(m_sdlWindow, title.c_str());
}
