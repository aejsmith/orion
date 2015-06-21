/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Debug manager.
 */

#pragma once

#include "core/engine_global.h"

#include "engine/font.h"
#include "engine/material.h"

class DebugOverlay;
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

    /** @return             Debug text material. */
    Material *textMaterial() const { return m_textMaterial; }
    /** @return             Debug text font. */
    Font *textFont() const { return m_textFont; }
    /** @return             Debug text font variant. */
    FontVariant *textFontVariant() const { return m_textFontVariant; }
private:
    /** Details of a line to draw. */
    struct Line {
        glm::vec3 start;                /**< Start of the line. */
        glm::vec3 end;                  /**< End of the line. */
        glm::vec4 colour;               /**< Colour to draw the line in. */
    };
private:
    MaterialPtr m_primitiveMaterial;    /**< Material for drawing debug primitives. */
    MaterialPtr m_textMaterial;         /**< Material for drawing debug text. */
    FontPtr m_textFont;                 /**< Font for debug text. */
    FontVariant *m_textFontVariant;     /**< Font variant for debug text. */

    DebugOverlay *m_overlay;            /**< Debug overlay. */

    std::vector<Line> m_perFrameLines;  /**< Lines to draw for all views in the frame. */
    std::vector<Line> m_perViewLines;   /**< Lines to draw for the next view only. */
};

extern EngineGlobal<DebugManager> g_debugManager;
