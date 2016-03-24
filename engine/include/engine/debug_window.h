/*
 * Copyright (C) 2016 Alex Smith
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
 * @brief               Debug window class.
 */

#pragma once

#include "core/core.h"

#include <imgui.h>

/**
 * Debug window class.
 *
 * This class provides an API for adding a window into the debug overlay GUI.
 * The window will be added to the main menu which appears when the GUI is
 * active, and when visible its render() method will be called, within which
 * ImGUI methods can be used to create the window.
 */
class DebugWindow {
public:
    /** Destroy a window. */
    virtual ~DebugWindow() {}
protected:
    /** Initialise a debug window.
     * @param title         Title for the window. */
    explicit DebugWindow(const std::string &title) :
        m_title(title),
        m_open(false)
    {}

    /**
     * Render the window's contents.
     *
     * Only called if the window contents should actually be drawn. Must call
     * ImGui::Begin() and ImGui::End() (the begin() wrapper provided will call
     * with the appropriate arguments).
     */
    virtual void render() = 0;

    /** Wrapper for ImGUI's Begin() method.
     * @param flags         Window flags to pass through (default 0).
     * @return              Whether to draw the window. */
    virtual bool begin(ImGuiWindowFlags flags = 0) {
        return ImGui::Begin(m_title.c_str(), &m_open, flags);
    }
protected:
    std::string m_title;            /**< Window title. */
    bool m_open;                    /**< Whether the window is open. */

    friend class DebugOverlay;
};
