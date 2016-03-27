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
 * @brief               Debug manager.
 */

#pragma once

#include "engine/font.h"

#include "shader/material.h"

class DebugOverlay;
class DebugWindow;
class SceneView;

/**
 * Debugging drawing/HUD API.
 *
 * This class provides an API for drawing lines etc. for debugging purposes.
 * Any primitives drawn with this API between the start of a frame and the time
 * a view is rendered will be rendered into the scene.
 */
class DebugManager : Noncopyable {
public:
    DebugManager();
    ~DebugManager();

    void initResources();

    void drawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &colour, bool perView = false);

    void writeText(const std::string &text, const glm::vec4 &colour = glm::vec4(1.0));

    void renderView(SceneView *view);

    void startFrame();
    void endFrame();

    void registerWindow(DebugWindow *window);
    void unregisterWindow(DebugWindow *window);
private:
    /** Details of a line to draw. */
    struct Line {
        glm::vec3 start;                /**< Start of the line. */
        glm::vec3 end;                  /**< End of the line. */
        glm::vec4 colour;               /**< Colour to draw the line in. */
    };
private:
    MaterialPtr m_primitiveMaterial;    /**< Material for drawing debug primitives. */

    DebugOverlay *m_overlay;            /**< Debug overlay. */

    std::vector<Line> m_perFrameLines;  /**< Lines to draw for all views in the frame. */
    std::vector<Line> m_perViewLines;   /**< Lines to draw for the next view only. */

    friend class DebugWindow;
};

extern DebugManager *g_debugManager;
