/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Rendering utility functions.
 */

#include "render/render_manager.h"
#include "render/utility.h"
#include "render/vertex.h"

/**
 * Create a quad.
 *
 * Create a quad, centered at the origin and extending from -1 to +1 in the X
 * and Y directions. The created vertex data has positions, normals and a single
 * set of texture coordinates.
 *
 * @param vertices      Where to return created vertex data object.
 */
void RenderUtil::makeQuad(GPUVertexDataPtr &vertices) {
    std::vector<SimpleVertex> vb;
    vb.emplace_back(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    vb.emplace_back(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f));
    vb.emplace_back(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f));
    vb.emplace_back(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f));
    vb.emplace_back(glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f));
    vb.emplace_back(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f));

    GPUBufferArray buffers(1);
    buffers[0] = RenderUtil::buildGPUBuffer(GPUBuffer::kVertexBuffer, vb);
    vertices = g_gpu->createVertexData(vb.size(), g_renderManager->simpleVertexFormat(), buffers);
}

/**
 * Create a sphere.
 *
 * Create a sphere centered at the origin with a radius of 1. The created vertex
 * data has positions, normals and a single set of texture coordinates.
 *
 * @param rings         Number of rings.
 * @param sides         Number of sides.
 * @param vertices      Where to return created vertex data object.
 * @param indices       Where to return created index data object.
 */
void RenderUtil::makeSphere(unsigned rings, unsigned sides, GPUVertexDataPtr &vertices, GPUIndexDataPtr &indices) {
    /* Based on the code found here:
     * http://stackoverflow.com/questions/5988686/how-do-i-create-a-3d-sphere-in-opengl-using-visual-c */

    float R = 1.0f / (float)(rings - 1);
    float S = 1.0f / (float)(sides - 1);

    std::vector<SimpleVertex> vb;
    vb.reserve(rings * sides);

    for (unsigned r = 0; r < rings; r++) {
        for (unsigned s = 0; s < sides; s++) {
            float y = sin(-glm::half_pi<float>() + glm::pi<float>() * r * R);
            float x = cos(2 * glm::pi<float>() * s * S) * sin(glm::pi<float>() * r * R);
            float z = sin(2 * glm::pi<float>() * s * S) * sin(glm::pi<float>() * r * R);
            vb.emplace_back(glm::vec3(x, y, z), glm::vec3(x, y, z), glm::vec2(s * S, r * R));
        }
    }

    GPUBufferArray buffers(1);
    buffers[0] = RenderUtil::buildGPUBuffer(GPUBuffer::kVertexBuffer, vb);
    vertices = g_gpu->createVertexData(vb.size(), g_renderManager->simpleVertexFormat(), buffers);

    std::vector<uint16_t> ib;
    ib.reserve(rings * sides * 6);

    for (unsigned r = 0; r < rings - 1; r++) {
        for (unsigned s = 0; s < sides - 1; s++) {
            ib.push_back(r * sides + s);
            ib.push_back((r + 1) * sides + s);
            ib.push_back((r + 1) * sides + (s + 1));
            ib.push_back((r + 1) * sides + (s + 1));
            ib.push_back(r * sides + (s + 1));
            ib.push_back(r * sides + s);
        }
    }

    indices = g_gpu->createIndexData(
        RenderUtil::buildGPUBuffer(GPUBuffer::kIndexBuffer, ib),
        GPUIndexData::kUnsignedShortType,
        ib.size());
}

/**
 * Create a cone.
 *
 * Creates a cone with the point on the origin, pointing forward (down the
 * negative Z axis), with a base radius of 1 and a height of 1. Note this does
 * not currently generate valid normals or texture coordinates.
 *
 * @param baseVertices  Number of vertices around the base.
 * @param vertices      Where to return created vertex data object.
 * @param indices       Where to return created index data object.
 */
void RenderUtil::makeCone(unsigned baseVertices, GPUVertexDataPtr &vertices, GPUIndexDataPtr &indices) {
    std::vector<SimpleVertex> vb;
    vb.reserve(baseVertices + 1);

    /* Add the vertices. Head, then the base. */
    vb.emplace_back(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec2(0.0f));
    float delta = (2 * glm::pi<float>()) / baseVertices;
    for (unsigned i = 0; i < baseVertices; i++) {
        float angle = i * delta;
        float x = cosf(angle);
        float y = sinf(angle);
        float z = -1;
        vb.emplace_back(glm::vec3(x, y, z), glm::vec3(0.0f), glm::vec2(0.0f));
    }

    GPUBufferArray buffers(1);
    buffers[0] = RenderUtil::buildGPUBuffer(GPUBuffer::kVertexBuffer, vb);
    vertices = g_gpu->createVertexData(vb.size(), g_renderManager->simpleVertexFormat(), buffers);

    std::vector<uint16_t> ib;
    ib.reserve((3 * baseVertices) + (3 * (baseVertices - 2)));

    /* Add indices. Cone head to base, then the base. */
    for (unsigned i = 0; i < baseVertices; i++) {
        ib.emplace_back(0);
        ib.emplace_back((i % baseVertices) + 1);
        ib.emplace_back(((i + 1) % baseVertices) + 1);
    }
    for (unsigned i = 0; i < baseVertices - 2; i++) {
        ib.emplace_back(1);
        ib.emplace_back(i + 3);
        ib.emplace_back(i + 2);
    }

    indices = g_gpu->createIndexData(
        RenderUtil::buildGPUBuffer(GPUBuffer::kIndexBuffer, ib),
        GPUIndexData::kUnsignedShortType,
        ib.size());
}
