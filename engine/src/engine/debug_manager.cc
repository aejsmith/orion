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
 * @brief               Debug manager.
 */

#include "core/string.h"

#include "engine/asset_manager.h"
#include "engine/debug_manager.h"
#include "engine/engine.h"
#include "engine/render_target.h"
#include "engine/window.h"

#include "gpu/gpu_manager.h"

#include "render/primitive_renderer.h"

/** Global debug manager. */
EngineGlobal<DebugManager> g_debugManager;

/** Debug overlay. */
class DebugOverlay : public RenderLayer {
public:
    DebugOverlay();
    ~DebugOverlay();

    void render() override;
private:
    void drawText(unsigned x, unsigned y, const std::string &text, const glm::vec3 &colour = glm::vec3(1.0));
};

/** Initialise the debug overlay. */
DebugOverlay::DebugOverlay() {
    /* Add the overlay to the main window. */
    setLayerOrder(RenderLayer::kDebugOverlayLayerOrder);
    setRenderTarget(g_mainWindow);
    registerLayer();
}

/** Destroy the debug overlay. */
DebugOverlay::~DebugOverlay() {
    unregisterLayer();
}

/** Render the debug overlay. */
void DebugOverlay::render() {
    renderTarget()->set(&pixelViewport());

    /* Want to blend text with background, no depth test. */
    g_gpuManager->setBlendState<
        BlendFunc::kAdd,
        BlendFactor::kSourceAlpha,
        BlendFactor::kOneMinusSourceAlpha>();
    g_gpuManager->setDepthStencilState<ComparisonFunc::kAlways, false>();

    FontVariant *font = g_debugManager->textFontVariant();

    /* Display engine statistics. */
    const EngineStats &stats = g_engine->stats();
    drawText(10, 10, String::format("FPS: %.1f", stats.fps));
    drawText(10, 10 + font->height(), String::format("Frame time: %.0f ms", stats.frameTime * 1000.0f));
    drawText(10, 10 + (2 * font->height()), String::format("Draw calls: %u", stats.drawCalls));
}

/** Scale a pixel distance to clip space distance.
 * @param distance      Pixel distance.
 * @param max           Maximum coordinate (width or height).
 * @return              Vertex coordinate. */
static inline float vertexScale(unsigned distance, unsigned max) {
    return (2.0f / static_cast<float>(max)) * static_cast<float>(distance);
}

/** Draw text in the debug font.
 * @param x             Position of left-most edge of text.
 * @param y             Position of top-most edge of text.
 * @param text          Text to draw.
 * @param colour        Colour to draw in. */
void DebugOverlay::drawText(unsigned x, unsigned y, const std::string &text, const glm::vec3 &colour) {
    FontVariant *font = g_debugManager->textFontVariant();

    Material *material = g_debugManager->textMaterial();
    material->setValue("colour", colour);

    PrimitiveRenderer renderer;
    renderer.begin(PrimitiveType::kTriangleList, material);

    unsigned currX = x;
    unsigned currY = y;

    for (size_t i = 0; i < text.length(); i++) {
        if (text[i] == '\r') {
            currX = x;
        } else if (text[i] == '\n') {
            currY += font->height();
            currX = x;
        } else {
            const FontGlyph &glyph = font->getGlyph(text[i]);

            /* Calculate vertex positions. */
            float vertexX = vertexScale(currX + glyph.offsetX, pixelViewport().width) - 1.0f;
            float vertexY = 1.0f - vertexScale(currY + glyph.offsetY, pixelViewport().height);
            float vertexWidth = vertexScale(glyph.width, pixelViewport().width);
            float vertexHeight = vertexScale(glyph.height, pixelViewport().height);

            /* Calculate UVs. */
            const Texture2D *texture = font->texture();
            float uvX = static_cast<float>(glyph.x) / static_cast<float>(texture->width());
            float uvY = static_cast<float>(glyph.y) / static_cast<float>(texture->height());
            float uvWidth = static_cast<float>(glyph.width) / static_cast<float>(texture->width());
            float uvHeight = static_cast<float>(glyph.height) / static_cast<float>(texture->height());

            /* Draw a quad covering the glyph area. */
            renderer.addVertex(
                glm::vec3(vertexX, vertexY - vertexHeight, 0.0f),
                glm::vec3(0.0f),
                glm::vec2(uvX, uvY + uvHeight));
            renderer.addVertex(
                glm::vec3(vertexX + vertexWidth, vertexY - vertexHeight, 0.0f),
                glm::vec3(0.0f),
                glm::vec2(uvX + uvWidth, uvY + uvHeight));
            renderer.addVertex(
                glm::vec3(vertexX + vertexWidth, vertexY, 0.0f),
                glm::vec3(0.0f),
                glm::vec2(uvX + uvWidth, uvY));
            renderer.addVertex(
                glm::vec3(vertexX + vertexWidth, vertexY, 0.0f),
                glm::vec3(0.0f),
                glm::vec2(uvX + uvWidth, uvY));
            renderer.addVertex(
                glm::vec3(vertexX, vertexY, 0.0f),
                glm::vec3(0.0f),
                glm::vec2(uvX, uvY));
            renderer.addVertex(
                glm::vec3(vertexX, vertexY - vertexHeight, 0.0f),
                glm::vec3(0.0f),
                glm::vec2(uvX, uvY + uvHeight));

            currX += glyph.advance;
        }
    }

    renderer.draw(nullptr);
}

/** Initialise the debug manager. */
DebugManager::DebugManager() {
    /* Load debug primitive/text shaders. */
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/debug_primitive");
    m_primitiveMaterial = new Material(shader);
    shader = g_assetManager->load<Shader>("engine/shaders/internal/debug_text");
    m_textMaterial = new Material(shader);

    /* Load debug text font. */
    m_textFont = g_assetManager->load<Font>("engine/fonts/source_code_pro");
    check(m_textFont->isFixedWidth());

    FontVariantDesc desc;
    desc.pointSize = 9;
    m_textFontVariant = m_textFont->getVariant(desc);
    if (!m_textFontVariant)
        fatal("Failed to load debug font variant");

    m_textMaterial->setValue("font", TextureBasePtr(m_textFontVariant->texture()));

    /* Create the debug overlay. */
    m_overlay = new DebugOverlay;
}

/** Destroy the debug manager. */
DebugManager::~DebugManager() {
    delete m_overlay;
}

/** Draw a line in the world.
 * @param start         Start position of the line.
 * @param end           End position of the line.
 * @param colour        Colour to draw the line in.
 * @param perView       If true, the line will only be drawn for the next view
 *                      rendered. Otherwise, it will be drawn for all views
 *                      rendered within the current frame. */
void DebugManager::drawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &colour, bool perView) {
    Line line = { start, end, colour };

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
