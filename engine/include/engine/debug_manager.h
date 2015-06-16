/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Debug manager.
 */

#pragma once

#include "core/engine_global.h"

#include "engine/material.h"

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

    void drawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &colour, bool perView = false);

    void renderView(SceneView *view);
    void endFrame();
private:
    /** Details of a line to draw. */
    struct Line {
        glm::vec3 start;                /**< Start of the line. */
        glm::vec3 end;                  /**< End of the line. */
        glm::vec4 colour;               /**< Colour to draw the line in. */
    };
private:
    MaterialPtr m_primitiveMaterial;    /**< Material for drawing debug primitives. */

    std::vector<Line> m_perFrameLines;  /**< Lines to draw for all views in the frame. */
    std::vector<Line> m_perViewLines;   /**< Lines to draw for the next view only. */
};

extern EngineGlobal<DebugManager> g_debugManager;
