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

    /* Create dummy vertex data. */
    GPUVertexDataLayoutPtr vertexLayout = g_gpuManager->createVertexDataLayout(GPUVertexDataLayoutDesc());
    m_vertices = g_gpuManager->createVertexData(3, vertexLayout, GPUBufferArray());
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
    m_material->shader()->pass(Pass::Type::kBasic, 0)->setDrawState(nullptr);

    g_gpuManager->draw(PrimitiveType::kTriangleList, m_vertices, nullptr);

    g_gpuManager->endRenderPass();
}
