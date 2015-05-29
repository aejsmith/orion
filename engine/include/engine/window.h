/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Engine main window class.
 */

#pragma once

#include "engine/render_target.h"

struct EngineConfiguration;
struct SDL_Window;

/** Engine main window class. */
class Window : public RenderTarget {
public:
    Window(const EngineConfiguration &config);
    ~Window();

    void gpu(GPUTextureImageRef &ref) override;

    void setTitle(const std::string &title);

    /** @return             SDL window. */
    SDL_Window *sdlWindow() const { return m_sdlWindow; }
    /** @return             Width of the window (in pixels). */
    uint32_t width() const override { return m_width; }
    /** @return             Height of the window (in pixels). */
    uint32_t height() const override { return m_height; }
private:
    SDL_Window *m_sdlWindow;        /**< SDL window. */
    uint32_t m_width;               /**< Width of the window. */
    uint32_t m_height;              /**< Height of the window. */
};

extern EngineGlobal<Window> g_mainWindow;
