/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Simple primitive renderer.
 */

#include "gpu/gpu_manager.h"

#include "render/geometry.h"
#include "render/primitive_renderer.h"
#include "render/render_manager.h"
#include "render/scene_view.h"
#include "render/utility.h"

#include "shader/slots.h"

/** Initialise the renderer. */
PrimitiveRenderer::PrimitiveRenderer() :
    m_currentBatch(nullptr)
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
    checkMsg(m_drawList.empty(), "No more batches may be added after first draw");

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
 * @param view          Optional view to render with. This must be given if
 *                      any shaders used require view uniforms. */
void PrimitiveRenderer::draw(SceneView *view) {
    m_currentBatch = nullptr;

    if (m_drawList.empty()) {
        for (auto &batch : m_batches) {
            const BatchKey &key = batch.first;
            BatchData &data = batch.second;

            if (!data.vertices.size())
                continue;

            /* Generate vertex data. */
            GPUBufferArray buffers(1);
            buffers[0] = RenderUtil::buildGPUBuffer(
                GPUBuffer::kVertexBuffer,
                data.vertices,
                GPUBuffer::kStreamDrawUsage);

            data.gpu = g_gpuManager->createVertexData(
                data.vertices.size(),
                g_renderManager->simpleVertexFormat(),
                buffers);

            /* No longer require CPU-side data. */
            data.vertices.clear();

            /* Add to draw list. */
            Geometry geometry;
            geometry.vertices = data.gpu;
            geometry.indices = nullptr;
            geometry.primitiveType = key.type;
            m_drawList.addDrawCalls(geometry, key.material, nullptr, Pass::kBasicPass);
        }
    }

    if (view)
        g_gpuManager->bindUniformBuffer(UniformSlots::kViewUniforms, view->uniforms());

    /* Render all batches. */
    m_drawList.draw();
}
