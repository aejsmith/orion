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

#include "mesh_loader.h"

/** Construct the mesh loader. */
MeshLoader::MeshLoader() {}

/**
 * Add a vertex attribute.
 *
 * Adds a vertex attribute to the mesh. The data type is fixed for a given
 * semantic.
 *
 * @param semantic      Attribute semantic.
 * @param index         Attribute index.
 */
void MeshLoader::addAttribute(VertexAttribute::Semantic semantic, unsigned index) {
    m_attributes.emplace_back();
    Attribute &attribute = m_attributes.back();
    attribute.semantic   = semantic;
    attribute.index      = index;
}

/**
 * Add a vertex.
 *
 * Adds a new vertex to the mesh. The vertex data must be filled into the
 * returned structure by the caller. Vertices will be ordered in the order in
 * which they are specified with this function.
 *
 * @param outIndex      Where to store vertex index.
 */
MeshLoader::Vertex &MeshLoader::addVertex(size_t &outIndex) {
    outIndex = m_vertices.size();

    m_vertices.emplace_back();
    return m_vertices.back();
}

/**
 * Add a sub-mesh.
 *
 * Adds a new sub-mesh to the mesh. The returned descriptor structure must be
 * filled in with details of the sub-mesh.
 */
MeshLoader::SubMeshDesc &MeshLoader::addSubMesh() {
    m_subMeshes.emplace_back();
    return m_subMeshes.back();
}

/**
 * Create the mesh.
 *
 * This should be called once all details of the mesh have been filled in to
 * create the mesh and upload all of its data, and generate extra data (e.g.
 * tangents).
 *
 * @return              Created mesh.
 */
MeshPtr MeshLoader::createMesh() {
    if (!m_attributes.size()) {
        logError("%s: No attributes defined", m_path);
        return nullptr;
    } else if (!m_vertices.size()) {
        logError("%s: No vertices defined", m_path);
        return nullptr;
    } else if (!m_subMeshes.size()) {
        logError("%s: No sub-meshes defined", m_path);
        return nullptr;
    }

    MeshPtr mesh(new Mesh());

    mesh->setNumVertices(m_vertices.size());

    /* Add vertex attributes and upload data. */
    for (Attribute &attribute : m_attributes) {
        VertexAttribute::Type type;
        size_t components;
        bool normalised;
        void *data;

        switch (attribute.semantic) {
            case VertexAttribute::kPositionSemantic:
                check(attribute.index == 0);

                type       = VertexAttribute::kFloatType;
                components = 3;
                normalised = false;
                data       = &m_vertices[0].position;
                break;

            case VertexAttribute::kNormalSemantic:
                check(attribute.index == 0);

                type       = VertexAttribute::kFloatType;
                components = 3;
                normalised = false;
                data       = &m_vertices[0].normal;
                break;

            case VertexAttribute::kTexcoordSemantic:
                check(attribute.index == 0);

                type       = VertexAttribute::kFloatType;
                components = 2;
                normalised = false;
                data       = &m_vertices[0].texcoord;
                break;

            case VertexAttribute::kTangentSemantic:
                check(attribute.index == 0);

                type       = VertexAttribute::kFloatType;
                components = 4;
                normalised = false;
                data       = &m_vertices[0].tangent;
                break;

            default:
                fatal("Unhandled attribute semantic %d", attribute.semantic);

        }

        mesh->addAttribute(attribute.semantic,
                           attribute.index,
                           type,
                           normalised,
                           components,
                           data,
                           sizeof(m_vertices[0]));
    }

    /* Register all submeshes. */
    for (const SubMeshDesc &desc : m_subMeshes) {
        SubMesh &subMesh = mesh->addSubMesh();

        /* Add the material slot. If this name has already been added the
         * existing index is returned. */
        subMesh.material = mesh->addMaterial(desc.material);

        /* Create an index buffer. */
        subMesh.setIndices(desc.indices);

        /* Calculate the bounding box for the sub-mesh. */
        subMesh.boundingBox = calculateBoundingBox(desc.indices);

        logDebug("%s: Submesh %u: %u indices", m_path, mesh->numSubMeshes() - 1, desc.indices.size());
    }

    logDebug(
        "%s: %u vertices, %u submeshes, %u materials",
        m_path, m_vertices.size(), mesh->numSubMeshes(), mesh->numMaterials());

    return mesh;
}

/** Calculate a bounding box.
 * @param indices       Indices of the sub-mesh.
 * @return              Calculated bounding box. */
BoundingBox MeshLoader::calculateBoundingBox(const std::vector<uint16_t> &indices) const {
    BoundingBox boundingBox(glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX));

    for (uint16_t index : indices) {
        check(index < m_vertices.size());
        const Vertex &vertex = m_vertices[index];

        boundingBox.minimum = glm::min(boundingBox.minimum, vertex.position);
        boundingBox.maximum = glm::max(boundingBox.maximum, vertex.position);
    }

    return boundingBox;
}
