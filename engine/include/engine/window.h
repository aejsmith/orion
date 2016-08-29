/*
 * Copyright (C) 2015-2016 Alex Smith
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

#pragma once

#include "engine/render_target.h"

struct EngineConfiguration;
struct SDL_Window;

/** Engine main window class. */
class Window : public RenderTarget {
public:
    Window(const EngineConfiguration &config, uint32_t sdlFlags, PixelFormat format);
    ~Window();

    void getRenderTargetDesc(GPURenderTargetDesc &desc) const override;
    void getTextureImageRef(GPUTextureImageRef &ref) const override;

    void setTitle(const std::string &title);

    /** @return             SDL window. */
    SDL_Window *sdlWindow() const { return m_sdlWindow; }
protected:
    #ifdef ORION_BUILD_DEBUG
    std::string renderTargetName() const override;
    #endif
private:
    SDL_Window *m_sdlWindow;        /**< SDL window. */

};

extern Window *g_mainWindow;
