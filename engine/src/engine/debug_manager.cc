/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Debug manager.
 */

#include "engine/asset_manager.h"
#include "engine/debug_manager.h"

#include "gpu/gpu_manager.h"

#include "render/primitive_renderer.h"

/** Global debug manager. */
EngineGlobal<DebugManager> g_debugManager;

/** Initialise the debug manager. */
DebugManager::DebugManager() {
    /* Create a debug primitive material. */
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/debug_primitive");
    m_primitiveMaterial = new Material(shader);
}

/** Destroy the debug manager. */
DebugManager::~DebugManager() {}

/** Draw a line in the world.
 * @param start         Start position of the line.
 * @param end           End position of the line.
 * @param colour        Colour to draw the line in.
 * @param perView       If true, the line will only be drawn for the next view
 *                      rendered. Otherwise, it will be drawn for all views
 *                      rendered within the current frame. */
void DebugManager::drawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &colour, bool perView) {
    Line line{ start, end, colour };

    if (perView) {
        m_perViewLines.push_back(line);
    } else {
        m_perFrameLines.push_back(line);
    }
}

/**
 * Render debug primitives for a view.
 *
 * Renders all debug primitives added for the frame and the current view into
 * the current render target. GPU state is modified.
 *
 * @param view          View to render for.
 */
void DebugManager::renderView(SceneView *view) {
    PrimitiveRenderer renderer;

    g_gpuManager->setBlendState<BlendFunc::kAdd, BlendFactor::kSourceAlpha, BlendFactor::kOneMinusSourceAlpha>();
    g_gpuManager->setDepthStencilState<ComparisonFunc::kAlways, false>();

    /* Add all lines. */
    renderer.begin(PrimitiveType::kLineList, m_primitiveMaterial);
    for (const Line &line : m_perFrameLines) {
        renderer.addVertex(line.start, glm::vec3(0.0f), line.colour);
        renderer.addVertex(line.end, glm::vec3(0.0f), line.colour);
    }
    for (const Line &line : m_perViewLines) {
        renderer.addVertex(line.start, glm::vec3(0.0f), line.colour);
        renderer.addVertex(line.end, glm::vec3(0.0f), line.colour);
    }

    /* Draw them. */
    renderer.draw(view);

    /* Clear out per-view primitives. */
    m_perViewLines.clear();
}

/** Clear all debug primitives added for the current frame. */
void DebugManager::endFrame() {
    m_perFrameLines.clear();
    m_perViewLines.clear();
}
