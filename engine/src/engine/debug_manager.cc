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

    void addText(const std::string &text, const glm::vec3 &colour);

    void render() override;
private:
    /** Details of a block of text. */
    struct TextBlock {
        std::string text;
        glm::vec3 colour;
    };
private:
    /** Debug text blocks. */
    std::list<TextBlock> m_textBlocks;
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

/** Add a block of text to draw on the debug overlay.
 * @param text          Text to draw.
 * @param colour        Colour to draw text in. */
void DebugOverlay::addText(const std::string &text, const glm::vec3 &colour) {
    if (m_textBlocks.empty() || m_textBlocks.back().colour != colour)
        m_textBlocks.emplace_back();

    TextBlock &block = m_textBlocks.back();
    block.text += text;
    block.colour = colour;
}

/** Scale a pixel distance to clip space distance.
 * @param distance      Pixel distance.
 * @param max           Maximum coordinate (width or height).
 * @return              Vertex coordinate. */
static inline float vertexScale(unsigned distance, unsigned max) {
    return (2.0f / static_cast<float>(max)) * static_cast<float>(distance);
}

/** Render the debug overlay. */
void DebugOverlay::render() {
    if (m_textBlocks.empty())
        return;

    renderTarget()->set(&pixelViewport());

    /* Want to blend text with background, no depth test. */
    g_gpuManager->setBlendState<
        BlendFunc::kAdd,
        BlendFactor::kSourceAlpha,
        BlendFactor::kOneMinusSourceAlpha>();
    g_gpuManager->setDepthStencilState<ComparisonFunc::kAlways, false>();

    FontVariant *font = g_debugManager->textFontVariant();
    Material *material = g_debugManager->textMaterial();

    const unsigned kMargin = 10;
    unsigned currX = kMargin;
    unsigned currY = kMargin;

    for (const TextBlock &block : m_textBlocks) {
        material->setValue("colour", block.colour);

        PrimitiveRenderer renderer;
        renderer.begin(PrimitiveType::kTriangleList, material);

        for (size_t i = 0; i < block.text.length(); i++) {
            if (block.text[i] == '\r') {
                currX = kMargin;
                continue;
            } else if (block.text[i] == '\n') {
                currY += font->height();
                currX = kMargin;
                continue;
            }

            const FontGlyph &glyph = font->getGlyph(block.text[i]);

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

        renderer.draw(nullptr);
    }

    m_textBlocks.clear();
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
 * Write text to the debug overlay.
 *
 * Writes a block of text to the debug overlay. Text is drawn at the point the
 * debug overlay is rendered. This is done based on render layer/target
 * priorities. Any text added with this function after the overlay is rendered
 * will be drawn next frame.
 *
 * @param text          Text to write.
 * @param colour        Colour to write in.
 */
void DebugManager::writeText(const std::string &text, const glm::vec3 &colour) {
    m_overlay->addText(text, colour);
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
