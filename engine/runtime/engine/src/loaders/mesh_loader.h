/*
 * Copyright (C) 2017 Alex Smith
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
 * @brief               Mesh loader class.
 */

#pragma once

#include "engine/asset_loader.h"
#include "engine/mesh.h"

#include <list>
#include <vector>

/** Mesh loader base class. */
class MeshLoader : public AssetLoader {
public:
    // FIXME: objgen can't detect that a class has unimplemented pure virtuals.
    CLASS("constructable": false);

    /** Whether to automatically generate tangents. */
    PROPERTY() bool generateTangents;

protected:
    /** Attribute information. */
    struct Attribute {
        VertexAttribute::Semantic semantic;
        unsigned index;
    };

    /**
     * Structure containing loaded vertex data.
     *
     * The fields of this structure which contain valid data depends on the
     * attributes which have been added.
     */
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
        glm::vec4 tangent;
    };

    /** Submesh descriptor. */
    struct SubMeshDesc {
        std::string material;               /**< Material name. */
        std::vector<uint16_t> indices;      /**< Array of vertex indices to go into index buffer. */
    };

protected:
    MeshLoader();

    void addAttribute(VertexAttribute::Semantic semantic, unsigned index);
    Vertex &addVertex(size_t &outIndex);
    SubMeshDesc &addSubMesh();

    MeshPtr createMesh();

private:
    BoundingBox calculateBoundingBox(const std::vector<uint16_t> &indices) const;
    void calculateTangents();

private:
    std::list<Attribute> m_attributes;      /**< Array of attribute details. */
    std::vector<Vertex> m_vertices;         /**< Array of vertices to go into the vertex buffer. */
    std::list<SubMeshDesc> m_subMeshes;     /**< List of submeshes. */
};
