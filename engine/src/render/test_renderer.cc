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
 * @brief               Test renderer.
 */

#include "engine/asset_manager.h"
#include "engine/window.h"

#include "gpu/gpu_manager.h"

#include "render/test_renderer.h"
#include "render/utility.h"

/** Set to 1 to enable use of VBOs, 0 to use only shader constants. */
#define TEST_VBO        1

#if TEST_VBO
static const size_t kUsePass = 1;
#else
static const size_t kUsePass = 0;
#endif

/** Vertex data layout. */
struct Vertex {
    glm::vec2 position;
    glm::vec2 _pad;
    glm::vec4 colour;
};

/** Initialise the test renderer. */
TestRenderer::TestRenderer() :
    RenderLayer(kDebugOverlayPriority)
{
    setRenderTarget(g_mainWindow);
    registerRenderLayer();

    /* Load the shader. */
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/test_renderer");
    m_material = new Material(shader);

    /* Create a render pass. */
    GPURenderPassDesc passDesc(1);
    passDesc.colourAttachments[0].format = renderTarget()->format();
    passDesc.colourAttachments[0].loadOp = GPURenderLoadOp::kClear;
    m_renderPass = g_gpuManager->createRenderPass(std::move(passDesc));

    /* Create vertex data. */
    #if TEST_VBO
        GPUVertexDataLayoutDesc vertexLayoutDesc(1, 2);
        vertexLayoutDesc.bindings[0].stride = sizeof(Vertex);
        vertexLayoutDesc.attributes[0].semantic = VertexAttribute::kPositionSemantic;
        vertexLayoutDesc.attributes[0].index = 0;
        vertexLayoutDesc.attributes[0].type = VertexAttribute::kFloatType;
        vertexLayoutDesc.attributes[0].components = 2;
        vertexLayoutDesc.attributes[0].binding = 0;
        vertexLayoutDesc.attributes[0].offset = offsetof(Vertex, position);
        vertexLayoutDesc.attributes[1].semantic = VertexAttribute::kDiffuseSemantic;
        vertexLayoutDesc.attributes[1].index = 0;
        vertexLayoutDesc.attributes[1].type = VertexAttribute::kFloatType;
        vertexLayoutDesc.attributes[1].components = 4;
        vertexLayoutDesc.attributes[1].binding = 0;
        vertexLayoutDesc.attributes[1].offset = offsetof(Vertex, colour);
        GPUVertexDataLayoutPtr vertexLayout = g_gpuManager->createVertexDataLayout(std::move(vertexLayoutDesc));

        const std::vector<Vertex> vertices = {
            { glm::vec2(-0.3f,  0.4f), glm::vec2(), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
            { glm::vec2( 0.3f,  0.4f), glm::vec2(), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) },
            { glm::vec2( 0.0f, -0.4f), glm::vec2(), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) },
        };

        auto vertexDataDesc = GPUVertexDataDesc().
            setCount(3).
            setLayout(std::move(vertexLayout));
        vertexDataDesc.buffers[0] = RenderUtil::buildGPUBuffer(GPUBuffer::kVertexBuffer, vertices);
    #else
        GPUVertexDataLayoutPtr vertexLayout = g_gpuManager->createVertexDataLayout(GPUVertexDataLayout());
        auto vertexDataDesc = GPUVertexDataDesc().
            setCount(3).
            setLayout(std::move(vertexLayout));
    #endif

    m_vertices = g_gpuManager->createVertexData(std::move(vertexDataDesc));
}

/** Destroy the test renderer. */
TestRenderer::~TestRenderer() {
    unregisterRenderLayer();
}

/** Render the scene.
 * @param first         Whether this is the first layer on the render target. */
void TestRenderer::render(bool first) {
    GPURenderPassInstanceDesc instanceDesc(m_renderPass);
    renderTarget()->getRenderTargetDesc(instanceDesc.targets);
    instanceDesc.clearColours[0] = glm::vec4(0.0, 0.0, 0.5, 1.0);
    instanceDesc.renderArea = pixelViewport();
    g_gpuManager->beginRenderPass(instanceDesc);

    m_material->setDrawState();
    m_material->shader()->pass(Pass::Type::kBasic, kUsePass)->setDrawState(nullptr);

    g_gpuManager->draw(PrimitiveType::kTriangleList, m_vertices, nullptr);

    g_gpuManager->endRenderPass();
}
