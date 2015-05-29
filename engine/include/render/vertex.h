/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Standard vertex format definitions.
 */

#pragma once

#include "core/core.h"

/**
 * Simple vertex structure.
 *
 * Simple vertex structure providing a position, normal, and a single set of
 * texture coordinates. The GPU vertex format matching this structure is
 * available from RenderManager::simpleVertexFormat().
 */
struct SimpleVertex {
    float x, y, z, _pad1;
    float nx, ny, nz, _pad2;
    float u, v, _pad3, _pad4;
public:
    /** Construct the vertex.
     * @param pos           Position.
     * @param normal        Normal.
     * @param texcoord      Texture coordinates. */
    SimpleVertex(const glm::vec3 &pos, const glm::vec3 &normal, const glm::vec2 &texcoord) :
        x(pos.x), y(pos.y), z(pos.z),
        nx(normal.x), ny(normal.y), nz(normal.z),
        u(texcoord.x), v(texcoord.y)
    {}
};
