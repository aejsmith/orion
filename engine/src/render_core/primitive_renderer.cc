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
 * @brief               Simple primitive renderer.
 */

#include "gpu/gpu_manager.h"

#include "render_core/geometry.h"
#include "render_core/primitive_renderer.h"
#include "render_core/render_resources.h"
#include "render_core/utility.h"

/** Initialise the renderer. */
PrimitiveRenderer::PrimitiveRenderer() :
    m_currentBatch(nullptr),
    m_finalized(false)
{}

/** Destroy the renderer. */
PrimitiveRenderer::~PrimitiveRenderer() {}

/**
 * Begin a new batch.
 *
 * This function sets state for the following calls to addVertex(). The type of
 * the primitives must be specified, along with a material to render them with.
 *
 * @param type          Primitive type to render.
 * @param material      Material to draw with (see class description for
 *                      limitations).
 */
void PrimitiveRenderer::begin(PrimitiveType type, Material *material) {
    checkMsg(!m_finalized, "No more batches may be added after first draw");

    BatchKey key = { type, material };
    auto it = m_batches.find(key);
    if (it != m_batches.end()) {
        m_currentBatch = &it->second;
    } else {
        auto ret = m_batches.insert(std::make_pair(key, BatchData()));
        m_currentBatch = &ret.first->second;
    }
}

/** Add a vertex to the current batch.
 * @param vertex        Vertex to add. */
void PrimitiveRenderer::doAddVertex(const SimpleVertex &vertex) {
    checkMsg(m_currentBatch, "Must begin a batch before adding vertices");

    m_currentBatch->vertices.push_back(vertex);
}

/** Draw all primitives that have been added.
 * @param cmdList       GPU command list.
 * @param view          Optional view resources to bind. This must be given if
 *                      any shaders used require view resources. */
void PrimitiveRenderer::draw(GPUCommandList *cmdList, GPUResourceSet *view) {
    m_currentBatch = nullptr;

    if (!m_finalized) {
        /* Generate GPU buffers. */
        for (auto &batch : m_batches) {
            BatchData &data = batch.second;

            if (!data.vertices.size())
                continue;

            /* Generate vertex data. */
            auto vertexDataDesc = GPUVertexDataDesc().
                setCount(data.vertices.size()).
                setLayout(g_renderResources->simpleVertexDataLayout());
            vertexDataDesc.buffers[0] = RenderUtil::buildGPUBuffer(
                GPUBuffer::kVertexBuffer,
                data.vertices,
                GPUBuffer::kTransientUsage);
            data.gpu = g_gpuManager->createVertexData(std::move(vertexDataDesc));

            /* No longer require CPU-side data. */
            data.vertices.clear();
        }

        m_finalized = true;
    }

    if (view)
        cmdList->bindResourceSet(ResourceSets::kViewResources, view);

    /* Render all batches. */
    for (auto &batch : m_batches) {
        const BatchKey &key = batch.first;
        const BatchData &data = batch.second;

        key.material->setDrawState(cmdList, Pass::Type::kBasic, 0);
        cmdList->draw(key.type, data.gpu, nullptr);
    }
}
