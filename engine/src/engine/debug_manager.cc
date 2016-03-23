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

#include "core/string.h"

#include "engine/asset_manager.h"
#include "engine/debug_manager.h"
#include "engine/engine.h"
#include "engine/render_target.h"
#include "engine/window.h"

#include "gpu/gpu_manager.h"

#include "input/input_handler.h"
#include "input/input_manager.h"

#include "render/primitive_renderer.h"

#include <imgui/imgui.h>

#include <SDL.h>

/** Global debug manager. */
EngineGlobal<DebugManager> g_debugManager;

/** Debug GUI overlay. */
class DebugOverlay : public RenderLayer {
public:
    DebugOverlay();
    ~DebugOverlay();

    void addText(const std::string &text, const glm::vec4 &colour);

    void startFrame();
    void render() override;
private:
    static const char *getClipboardText();
    static void setClipboardText(const char *text);
private:
    GPUVertexFormatPtr m_vertexFormat;  /**< Vertex format for GUI drawing. */
    MaterialPtr m_material;             /**< Material for GUI drawing. */
    Texture2DPtr m_texture;             /**< Font texture for GUI. */
};

/** Initialise the debug overlay. */
DebugOverlay::DebugOverlay() :
    RenderLayer(RenderLayer::kDebugOverlayPriority),
{
    ImGuiIO &io = ImGui::GetIO();
    io.SetClipboardTextFn = setClipboardText;
    io.GetClipboardTextFn = getClipboardText;
    io.IniFilename = nullptr;

    /* Map needed key codes. Text input is handled outside of this. */
    io.KeyMap[ImGuiKey_Tab] = static_cast<int>(InputCode::kTab);
    io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(InputCode::kLeft);
    io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(InputCode::kRight);
    io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(InputCode::kUp);
    io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(InputCode::kDown);
    io.KeyMap[ImGuiKey_PageUp] = static_cast<int>(InputCode::kPageUp);
    io.KeyMap[ImGuiKey_PageDown] = static_cast<int>(InputCode::kPageDown);
    io.KeyMap[ImGuiKey_Home] = static_cast<int>(InputCode::kHome);
    io.KeyMap[ImGuiKey_End] = static_cast<int>(InputCode::kEnd);
    io.KeyMap[ImGuiKey_Delete] = static_cast<int>(InputCode::kDelete);
    io.KeyMap[ImGuiKey_Backspace] = static_cast<int>(InputCode::kBackspace);
    io.KeyMap[ImGuiKey_Enter] = static_cast<int>(InputCode::kReturn);
    io.KeyMap[ImGuiKey_Escape] = static_cast<int>(InputCode::kEscape);
    io.KeyMap[ImGuiKey_A] = static_cast<int>(InputCode::kA);
    io.KeyMap[ImGuiKey_C] = static_cast<int>(InputCode::kC);
    io.KeyMap[ImGuiKey_V] = static_cast<int>(InputCode::kV);
    io.KeyMap[ImGuiKey_X] = static_cast<int>(InputCode::kX);
    io.KeyMap[ImGuiKey_Y] = static_cast<int>(InputCode::kY);
    io.KeyMap[ImGuiKey_Z] = static_cast<int>(InputCode::kZ);

    /* Create the ImGUI vertex format. */
    VertexBufferLayoutArray bufferLayouts(1);
    bufferLayouts[0].stride = sizeof(ImDrawVert);
    VertexAttributeArray attributes(3);
    attributes[0].semantic = VertexAttribute::kPositionSemantic;
    attributes[0].index = 0;
    attributes[0].type = VertexAttribute::kFloatType;
    attributes[0].count = 2;
    attributes[0].buffer = 0;
    attributes[0].offset = offsetof(ImDrawVert, pos);
    attributes[1].semantic = VertexAttribute::kTexcoordSemantic;
    attributes[1].index = 0;
    attributes[1].type = VertexAttribute::kFloatType;
    attributes[1].count = 2;
    attributes[1].buffer = 0;
    attributes[1].offset = offsetof(ImDrawVert, uv);
    attributes[2].semantic = VertexAttribute::kDiffuseSemantic;
    attributes[2].index = 0;
    attributes[2].type = VertexAttribute::kUnsignedByteType;
    attributes[2].normalised = true;
    attributes[2].count = 4;
    attributes[2].buffer = 0;
    attributes[2].offset = offsetof(ImDrawVert, col);
    m_vertexFormat = g_gpuManager->createVertexFormat(bufferLayouts, attributes);

    /* Load GUI shader. */
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/debug_overlay");
    m_material = new Material(shader);

    /* Upload the font atlas. */
    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
    m_texture = new Texture2D(width, height, PixelFormat::kR8, 1);
    m_texture->update(pixels);
    io.Fonts->SetTexID(m_texture.get());
    io.Fonts->ClearTexData();

    /* Add the overlay to the main window. */
    setRenderTarget(g_mainWindow);
    registerRenderLayer();
}

/** Destroy the debug overlay. */
DebugOverlay::~DebugOverlay() {
    unregisterRenderLayer();
}

/** Add a block of text to draw on the debug overlay.
 * @param text          Text to draw.
 * @param colour        Colour to draw text in. */
void DebugOverlay::addText(const std::string &text, const glm::vec4 &colour) {
    // FIXME: Don't do anything if we've disabled the overlay

    /* Creating a window with the same title appends to it. */
    const IntRect &viewport = pixelViewport();
    ImGui::SetNextWindowSize(ImVec2(viewport.width - 10, viewport.height - 10));
    ImGui::SetNextWindowPos(ImVec2(5, 5));
    ImGui::Begin(
        "Debug Text",
        nullptr,
        ImVec2(0, 0),
        0.0f,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::PushStyleColor(ImGuiCol_Text, colour);
    ImGui::Text(text.c_str());
    ImGui::PopStyleColor();
    ImGui::End();
}

/** Begin the frame. */
void DebugOverlay::startFrame() {
    ImGuiIO &io = ImGui::GetIO();

    /* Update viewport. */
    ImVec2 size(pixelViewport().width, pixelViewport().height);
    if (size.x != io.DisplaySize.x || size.y != io.DisplaySize.y) {
        /* Set up a new projection matrix. */
        glm::mat4 projectionMatrix(
            2.0f / size.x, 0.0f,           0.0f,  0.0f,
            0.0f,          2.0f / -size.y, 0.0f,  0.0f,
            0.0f,          0.0f,           -1.0f, 0.0f,
            -1.0f,         1.0f,           0.0f,  1.0f);
        m_material->setValue("projectionMatrix", projectionMatrix);

        io.DisplaySize = size;
    }

    // FIXME: This is not quite accurate.
    io.DeltaTime = g_engine->stats().frameTime;

    ImGui::NewFrame();
}

/** Render the debug overlay. */
void DebugOverlay::render() {
    /* Render the GUI. */
    ImGui::Render();
    const ImDrawData *drawData = ImGui::GetDrawData();
    if (!drawData)
        return;

    const IntRect &viewport = pixelViewport();

    renderTarget()->set(&viewport);

    g_gpuManager->setBlendState<BlendFunc::kAdd, BlendFactor::kSourceAlpha, BlendFactor::kOneMinusSourceAlpha>();
    g_gpuManager->setDepthStencilState<ComparisonFunc::kAlways, false>();
    g_gpuManager->setRasterizerState<CullMode::kDisabled>();

    for (int i = 0; i < drawData->CmdListsCount; i++) {
        const ImDrawList *cmdList = drawData->CmdLists[i];

        /* Generate vertex data. */
        size_t vertexBufferSize = cmdList->VtxBuffer.size() * sizeof(ImDrawVert);
        GPUBufferArray vertexBuffers(1);
        vertexBuffers[0] = g_gpuManager->createBuffer(
            GPUBuffer::kVertexBuffer,
            GPUBuffer::kStreamDrawUsage,
            vertexBufferSize);
        vertexBuffers[0]->write(0, vertexBufferSize, &cmdList->VtxBuffer.front());
        GPUVertexDataPtr vertexData = g_gpuManager->createVertexData(
            cmdList->VtxBuffer.size(),
            m_vertexFormat,
            vertexBuffers);

        /* Generate index buffer. */
        size_t indexBufferSize = cmdList->IdxBuffer.size() * sizeof(ImDrawIdx);
        GPUBufferPtr indexBuffer = g_gpuManager->createBuffer(
            GPUBuffer::kIndexBuffer,
            GPUBuffer::kStreamDrawUsage,
            indexBufferSize);
        indexBuffer->write(0, indexBufferSize, &cmdList->IdxBuffer.front());

        size_t indexBufferOffset = 0;
        for (const ImDrawCmd *cmd = cmdList->CmdBuffer.begin(); cmd != cmdList->CmdBuffer.end(); cmd++) {
            TextureBasePtr texture(reinterpret_cast<TextureBase *>(cmd->TextureId));
            m_material->setValue("tex", texture);

            /* Create index data. */
            GPUIndexDataPtr indexData = g_gpuManager->createIndexData(
                indexBuffer,
                GPUIndexData::kUnsignedShortType,
                cmd->ElemCount,
                indexBufferOffset);

            /* Configure scissor test for clipping. */
            g_gpuManager->setScissor(
                true,
                IntRect(
                    viewport.x + cmd->ClipRect.x,
                    viewport.y + (viewport.height - cmd->ClipRect.w),
                    cmd->ClipRect.z - cmd->ClipRect.x,
                    cmd->ClipRect.w - cmd->ClipRect.y));

            /* Prepare a draw call. */
            Geometry geometry;
            geometry.vertices = vertexData;
            geometry.indices = indexData;
            geometry.primitiveType = PrimitiveType::kTriangleList;
            DrawList drawList;
            drawList.addDrawCalls(geometry, m_material, nullptr, Pass::kBasicPass);
            drawList.draw();

            indexBufferOffset += cmd->ElemCount * sizeof(ImDrawIdx);
        }
    }

    // FIXME: this should be reset elsewhere
    g_gpuManager->setScissor(false, IntRect());
}

/** Get the current clipboard text. */
const char *DebugOverlay::getClipboardText() {
    return SDL_GetClipboardText();
}

/** Set the current clipboard text. */
void DebugOverlay::setClipboardText(const char *text) {
    SDL_SetClipboardText(text);
}

/** Initialise the debug manager. */
DebugManager::DebugManager() {
    /* Load debug primitive shader. */
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/debug_primitive");
    m_primitiveMaterial = new Material(shader);

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
void DebugManager::writeText(const std::string &text, const glm::vec4 &colour) {
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

/** Begin the frame. */
void DebugManager::startFrame() {
    m_overlay->startFrame();
}

/** Clear all debug primitives added for the current frame. */
void DebugManager::endFrame() {
    m_perFrameLines.clear();
    m_perViewLines.clear();
}
