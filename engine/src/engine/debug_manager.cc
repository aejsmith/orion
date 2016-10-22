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
#include "engine/debug_window.h"
#include "engine/engine.h"
#include "engine/render_target.h"
#include "engine/window.h"

#include "gpu/gpu_manager.h"

#include "input/input_handler.h"
#include "input/input_manager.h"

#include "render/primitive_renderer.h"

#include <SDL.h>

/** Global debug manager. */
DebugManager *g_debugManager;

/** Debug GUI overlay. */
class DebugOverlay : public RenderLayer, public InputHandler {
public:
    DebugOverlay();
    ~DebugOverlay();

    void initResources();

    void addText(const std::string &text, const glm::vec4 &colour);

    void startFrame();

    void registerWindow(std::unique_ptr<DebugWindow> window);
protected:
    void render(bool first) override;

    bool handleButtonDown(const ButtonEvent &event) override;
    bool handleButtonUp(const ButtonEvent &event) override;
    bool handleAxis(const AxisEvent &event) override;
    void handleTextInput(const TextInputEvent &event) override;

    #ifdef ORION_BUILD_DEBUG
    std::string renderLayerName() const override;
    #endif
private:
    /** State of the GUI. */
    enum class State {
        kInactive,                      /**< Not visible. */
        kVisible,                       /**< Visible, not receiving input. */
        kActive,                        /**< Visible, with input captured. */
    };
private:
    static const char *getClipboardText();
    static void setClipboardText(const char *text);
private:
    /** Vertex data layout for GUI drawing. */
    GPUVertexDataLayoutPtr m_vertexDataLayout;

    MaterialPtr m_material;             /**< Material for GUI drawing. */
    GPUTexturePtr m_fontTexture;        /**< Font texture for GUI. */
    GPUSamplerStatePtr m_sampler;       /**< Texture sampler for the GUI. */
    State m_state;                      /**< State of the GUI. */
    bool m_previousMouseCapture;        /**< Previous mouse capture state. */
    bool m_inputtingText;               /**< Whether currently inputting text. */

    /** List of GUI windows. */
    std::list<std::unique_ptr<DebugWindow>> m_windows;

    /** List of texture references. */
    std::list<GPUTexturePtr> m_textures;

    friend class DebugWindow;
};

/**
 * Create an ImGUI texture ID from a GPU texture.
 *
 * Creates a texture ID that can be passed to ImGUI's image functions from
 * a GPU texture. As part of this the texture is referenced to ensure it is
 * not destroyed before it is drawn. The reference will be released after
 * the debug overlay has been rendered.
 *
 * This function should only be used from within a render() method, as any
 * references created will not get released if the debug overlay is not
 * rendered.
 *
 * @param texture       Texture to reference.
 *
 * @return              Created texture ID.
 */
ImTextureID DebugWindow::refTexture(GPUTexture *texture) {
    /* This adds a reference as we hold a list of GPUTexturePtrs. */
    g_debugManager->m_overlay->m_textures.emplace_back(texture);
    return texture;
}

/** Create an ImGUI texture ID from a texture asset.
 * @see                 refTexture(GPUTexture *)
 * @param texture       Texture to create from.
 * @return              Created texture ID. */
ImTextureID DebugWindow::refTexture(Texture2D *texture) {
    return refTexture(texture->gpu());
}

/** Initialise the debug overlay. */
DebugOverlay::DebugOverlay() :
    RenderLayer(RenderLayer::kDebugOverlayPriority),
    InputHandler(InputHandler::kDebugOverlayPriority),
    m_state(State::kInactive),
    m_inputtingText(false)
{}

/** Destroy the debug overlay. */
DebugOverlay::~DebugOverlay() {
    unregisterRenderLayer();
}

/** Create resources for the debug overlay. */
void DebugOverlay::initResources() {
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

    /* Create the ImGUI vertex data layout. */
    GPUVertexDataLayoutDesc vertexDesc(1, 3);
    vertexDesc.bindings[0].stride = sizeof(ImDrawVert);
    vertexDesc.attributes[0].semantic = VertexAttribute::kPositionSemantic;
    vertexDesc.attributes[0].index = 0;
    vertexDesc.attributes[0].type = VertexAttribute::kFloatType;
    vertexDesc.attributes[0].components = 2;
    vertexDesc.attributes[0].binding = 0;
    vertexDesc.attributes[0].offset = offsetof(ImDrawVert, pos);
    vertexDesc.attributes[1].semantic = VertexAttribute::kTexcoordSemantic;
    vertexDesc.attributes[1].index = 0;
    vertexDesc.attributes[1].type = VertexAttribute::kFloatType;
    vertexDesc.attributes[1].components = 2;
    vertexDesc.attributes[1].binding = 0;
    vertexDesc.attributes[1].offset = offsetof(ImDrawVert, uv);
    vertexDesc.attributes[2].semantic = VertexAttribute::kDiffuseSemantic;
    vertexDesc.attributes[2].index = 0;
    vertexDesc.attributes[2].type = VertexAttribute::kUnsignedByteType;
    vertexDesc.attributes[2].normalised = true;
    vertexDesc.attributes[2].components = 4;
    vertexDesc.attributes[2].binding = 0;
    vertexDesc.attributes[2].offset = offsetof(ImDrawVert, col);
    m_vertexDataLayout = g_gpuManager->createVertexDataLayout(std::move(vertexDesc));

    /* Load GUI shader. */
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/debug_overlay");
    m_material = new Material(shader);

    /* Upload the font atlas. TODO: We're currently using an RGBA32 texture even
     * though only the alpha channel is used by the font atlas. This is so that
     * the shader is compatible with other textures. This is wasteful, the
     * better thing to do in future would be to use a R8 texture and then use
     * texture swizzling parameters to shift that into the alpha position (and
     * make all other components 1) when sampling. */
    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    auto textureDesc = GPUTextureDesc().
        setType(GPUTexture::kTexture2D).
        setWidth(width).
        setHeight(height).
        setFormat(PixelFormat::kR8G8B8A8).
        setMips(1);
    m_fontTexture = g_gpuManager->createTexture(textureDesc);
    m_fontTexture->update(
        IntRect(0, 0, width, height),
        pixels);
    io.Fonts->SetTexID(m_fontTexture.get());
    io.Fonts->ClearTexData();

    /* Create the texture sampler. */
    auto samplerDesc = GPUSamplerStateDesc().
        setFilterMode(SamplerFilterMode::kBilinear).
        setMaxAnisotropy(1);
    m_sampler = g_gpuManager->getSamplerState(samplerDesc);

    /* Add the overlay to the main window. */
    setRenderTarget(g_mainWindow);
    registerRenderLayer();
    registerInputHandler();
}

/** Add a block of text to draw on the debug overlay.
 * @param text          Text to draw.
 * @param colour        Colour to draw text in. */
void DebugOverlay::addText(const std::string &text, const glm::vec4 &colour) {
    // FIXME: Don't do anything if we've disabled the overlay

    /* Calculate dimensions of the text window. Avoid the menu bar if visible. */
    const IntRect &viewport = pixelViewport();
    int delta = (m_state >= State::kActive) ? 20 : 0;
    ImGui::SetNextWindowSize(ImVec2(viewport.width - 20, viewport.height - 20 - delta));
    ImGui::SetNextWindowPos(ImVec2(10, 10 + delta));

    /* Creating a window with the same title appends to it. */
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

    /* Pass input state. */
    io.MousePos = (m_state >= State::kActive) ? g_inputManager->getCursorPosition() : ImVec2(-1, -1);
    uint32_t modifiers = (m_state >= State::kActive) ? g_inputManager->getModifiers() : 0;
    io.KeyShift = (modifiers & InputModifier::kShift) != 0;
    io.KeyCtrl = (modifiers & InputModifier::kCtrl) != 0;
    io.KeyAlt = (modifiers & InputModifier::kAlt) != 0;

    ImGui::NewFrame();

    if (io.WantTextInput != m_inputtingText) {
        if (io.WantTextInput) {
            beginTextInput();
        } else {
            endTextInput();
        }

        m_inputtingText = io.WantTextInput;
    }
}

/** Render the debug overlay.
 * @param first         Whether this is the first layer on the RT. */
void DebugOverlay::render(bool first) {
    if (m_state >= State::kVisible) {
        if (m_state >= State::kActive) {
            /* Draw the main menu. */
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Windows")) {
                    for (auto &window : m_windows) {
                        if (ImGui::MenuItem(window->m_title.c_str(), "", window->m_open))
                            window->m_open = !window->m_open;
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }
        }

        /* Draw debug windows. */
        for (auto &window : m_windows) {
            if (window->m_open)
                window->render();
        }
    }

    /* Render the GUI. Always done because text overlay is independent of the
     * main GUI. */
    ImGui::Render();
    const ImDrawData *drawData = ImGui::GetDrawData();
    if (!drawData)
        return;

    GPUCommandList *cmdList = beginLayerRenderPass(
        (first) ? GPURenderLoadOp::kClear : GPURenderLoadOp::kLoad,
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    cmdList->setBlendState(GPUBlendStateDesc().
        setFunc(BlendFunc::kAdd).
        setSourceFactor(BlendFactor::kSourceAlpha).
        setDestFactor(BlendFactor::kOneMinusSourceAlpha));
    cmdList->setDepthStencilState(GPUDepthStencilStateDesc().
        setDepthFunc(ComparisonFunc::kAlways).
        setDepthWrite(false));
    cmdList->setRasterizerState(GPURasterizerStateDesc().
        setCullMode(CullMode::kDisabled));

    for (int i = 0; i < drawData->CmdListsCount; i++) {
        const ImDrawList *imCmdList = drawData->CmdLists[i];

        /* Generate vertex data. */
        auto vertexDataDesc = GPUVertexDataDesc().
            setCount(imCmdList->VtxBuffer.size()).
            setLayout(m_vertexDataLayout);
        auto vertexBufferDesc = GPUBufferDesc().
            setType(GPUBuffer::kVertexBuffer).
            setUsage(GPUBuffer::kTransientUsage).
            setSize(imCmdList->VtxBuffer.size() * sizeof(ImDrawVert));
        vertexDataDesc.buffers[0] = g_gpuManager->createBuffer(vertexBufferDesc);
        vertexDataDesc.buffers[0]->write(0, vertexBufferDesc.size, &imCmdList->VtxBuffer.front());
        GPUVertexDataPtr vertexData = g_gpuManager->createVertexData(std::move(vertexDataDesc));

        /* Generate index buffer. */
        auto indexBufferDesc = GPUBufferDesc().
            setType(GPUBuffer::kIndexBuffer).
            setUsage(GPUBuffer::kTransientUsage).
            setSize(imCmdList->IdxBuffer.size() * sizeof(ImDrawIdx));
        GPUBufferPtr indexBuffer = g_gpuManager->createBuffer(indexBufferDesc);
        indexBuffer->write(0, indexBufferDesc.size, &imCmdList->IdxBuffer.front());

        size_t indexBufferOffset = 0;
        for (const ImDrawCmd *cmd = imCmdList->CmdBuffer.begin(); cmd != imCmdList->CmdBuffer.end(); cmd++) {
            GPUTexture *texture = reinterpret_cast<GPUTexture *>(cmd->TextureId);
            m_material->setGPUTexture("debugTexture", texture, m_sampler);

            /* Create index data. */
            auto indexDataDesc = GPUIndexDataDesc().
                setBuffer(indexBuffer).
                setType(GPUIndexData::kUnsignedShortType).
                setCount(cmd->ElemCount).
                setOffset(indexBufferOffset);
            GPUIndexDataPtr indexData = g_gpuManager->createIndexData(std::move(indexDataDesc));

            /* Configure scissor test for clipping. */
            const IntRect &viewport = pixelViewport();
            cmdList->setScissor(
                true,
                IntRect(
                    viewport.x + cmd->ClipRect.x,
                    viewport.y + cmd->ClipRect.y,
                    cmd->ClipRect.z - cmd->ClipRect.x,
                    cmd->ClipRect.w - cmd->ClipRect.y));

            /* Prepare a draw call. */
            Geometry geometry;
            geometry.vertices = vertexData;
            geometry.indices = indexData;
            geometry.primitiveType = PrimitiveType::kTriangleList;
            DrawList drawList;
            drawList.addDrawCalls(geometry, m_material, nullptr, Pass::Type::kBasic);
            drawList.draw(cmdList);

            indexBufferOffset += cmd->ElemCount;
        }
    }

    g_gpuManager->submitRenderPass(cmdList);

    /* Release any texture references held. */
    m_textures.clear();
}

/** Register a debug GUI window.
 * @param window        Window to register. */
void DebugOverlay::registerWindow(std::unique_ptr<DebugWindow> window) {
    m_windows.emplace_back(std::move(window));
}

/** Handle a button down event.
 * @param event         Button event.
 * @return              Whether the event was handled. */
bool DebugOverlay::handleButtonDown(const ButtonEvent &event) {
    if (m_state >= State::kActive) {
        ImGuiIO &io = ImGui::GetIO();

        if (event.code < InputCode::kNumKeyboardCodes) {
            io.KeysDown[static_cast<int>(event.code)] = true;
        } else if (event.code < InputCode::kNumMouseCodes) {
            switch (event.code) {
                case InputCode::kMouseLeft:
                    io.MouseDown[0] = true;
                    break;
                case InputCode::kMouseRight:
                    io.MouseDown[1] = true;
                    break;
                case InputCode::kMouseMiddle:
                    io.MouseDown[2] = true;
                    break;
                default:
                    break;
            }
        }

        return true;
    }

    return false;
}

/** Handle a button up event.
 * @param event         Button event.
 * @return              Whether the event was handled. */
bool DebugOverlay::handleButtonUp(const ButtonEvent &event) {
    auto setState = [&] (State state) {
        if (m_state < State::kActive && state >= State::kActive) {
            /* Set the global mouse capture state to false because we want to
             * use the OS cursor. */
            m_previousMouseCapture = g_inputManager->mouseCaptured();
            g_inputManager->setMouseCaptured(false);
        } else if (m_state >= State::kActive && state < State::kActive) {
            g_inputManager->setMouseCaptured(m_previousMouseCapture);
        }

        m_state = state;
    };

    if (event.code == InputCode::kF1) {
        setState((m_state == State::kInactive) ? State::kActive : State::kInactive);
        return true;
    }

    if (m_state >= State::kVisible) {
        if (event.code == InputCode::kF2) {
            setState((m_state == State::kVisible) ? State::kActive : State::kVisible);
            return true;
        } else if (m_state >= State::kActive) {
            ImGuiIO &io = ImGui::GetIO();

            if (event.code < InputCode::kNumKeyboardCodes) {
                io.KeysDown[static_cast<int>(event.code)] = false;
            } else if (event.code < InputCode::kNumMouseCodes) {
                switch (event.code) {
                    case InputCode::kMouseLeft:
                        io.MouseDown[0] = false;
                        break;
                    case InputCode::kMouseRight:
                        io.MouseDown[1] = false;
                        break;
                    case InputCode::kMouseMiddle:
                        io.MouseDown[2] = false;
                        break;
                    default:
                        break;
                }
            }

            return true;
        }
    }

    return false;
}

/** Handle an axis event.
 * @param event         Axis event.
 * @return              Whether the event was handled. */
bool DebugOverlay::handleAxis(const AxisEvent &event) {
    if (m_state >= State::kActive) {
        ImGuiIO &io = ImGui::GetIO();

        switch (event.code) {
            case InputCode::kMouseScroll:
                io.MouseWheel = event.delta;
                break;
            default:
                break;
        }

        return true;
    }

    return false;
}

/** Handle a text input event.
 * @param event         Text input event. */
void DebugOverlay::handleTextInput(const TextInputEvent &event) {
    ImGuiIO &io = ImGui::GetIO();
    io.AddInputCharactersUTF8(event.text.c_str());
}

/** Get the current clipboard text. */
const char *DebugOverlay::getClipboardText() {
    return SDL_GetClipboardText();
}

/** Set the current clipboard text. */
void DebugOverlay::setClipboardText(const char *text) {
    SDL_SetClipboardText(text);
}

#ifdef ORION_BUILD_DEBUG

/** @return             Name of the layer (for debug purposes). */
std::string DebugOverlay::renderLayerName() const {
    return "DebugOverlay";
}

#endif

/** Initialise the debug manager. */
DebugManager::DebugManager() {
    m_overlay = new DebugOverlay;
}

/** Destroy the debug manager. */
DebugManager::~DebugManager() {
    delete m_overlay;
}

/** Create resources for the debug manager. */
void DebugManager::initResources() {
    /* Load debug primitive shader. */
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/debug_primitive");
    m_primitiveMaterial = new Material(shader);

    m_overlay->initResources();
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
 * @param cmdList       GPU command list.
 * @param view          View to render for.
 */
void DebugManager::renderView(GPUCommandList *cmdList, SceneView *view) {
    PrimitiveRenderer renderer;

    cmdList->setBlendState(GPUBlendStateDesc().
        setFunc(BlendFunc::kAdd).
        setSourceFactor(BlendFactor::kSourceAlpha).
        setDestFactor(BlendFactor::kOneMinusSourceAlpha));
    cmdList->setDepthStencilState(GPUDepthStencilStateDesc().
        setDepthFunc(ComparisonFunc::kAlways).
        setDepthWrite(false));

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
    renderer.draw(cmdList, view);

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

/** Register a debug GUI window.
 * @param window        Window to register. Will become owned by the manager. */
void DebugManager::registerWindow(std::unique_ptr<DebugWindow> window) {
    m_overlay->registerWindow(std::move(window));
}
