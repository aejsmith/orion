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
    float r, g, b, a;
public:
    /** Construct the vertex with a texture coordinate.
     * @param pos           Position.
     * @param normal        Normal.
     * @param texcoord      Texture coordinates. */
    SimpleVertex(const glm::vec3 &pos, const glm::vec3 &normal, const glm::vec2 &texcoord) :
        x(pos.x), y(pos.y), z(pos.z),
        nx(normal.x), ny(normal.y), nz(normal.z),
        u(texcoord.x), v(texcoord.y),
        r(0.0f), g(0.0f), b(0.0f), a(0.0f)
    {}

    /** Construct the vertex with a colour.
     * @param pos           Position.
     * @param normal        Normal.
     * @param colour        Colour. */
    SimpleVertex(const glm::vec3 &pos, const glm::vec3 &normal, const glm::vec4 &colour) :
        x(pos.x), y(pos.y), z(pos.z),
        nx(normal.x), ny(normal.y), nz(normal.z),
        u(0.0f), v(0.0f),
        r(colour.r), g(colour.g), b(colour.b), a(colour.a)
    {}
};
