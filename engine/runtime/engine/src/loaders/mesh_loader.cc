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
MeshLoader::MeshLoader() :
    generateTangents (false)
{}

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

    /* Calculate tangents if required and we do not have them. */
    calculateTangents();

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

    logDebug("%s: %u vertices, %u submeshes, %u materials",
             m_path,
             m_vertices.size(),
             mesh->numSubMeshes(),
             mesh->numMaterials());

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

/** Calculate tangents for the mesh if required and it does not have them. */
void MeshLoader::calculateTangents() {
    bool needTangents = this->generateTangents;

    if (needTangents) {
        for (Attribute &attribute : m_attributes) {
            if (attribute.semantic == VertexAttribute::kTangentSemantic) {
                needTangents = false;
                break;
            }
        }
    }

    if (!needTangents)
        return;

    /* Add an attribute for it. */
    addAttribute(VertexAttribute::kTangentSemantic, 0);

    /*
     * Tangent/bitangent vector calculation based on Eric Lengyel's method.
     * Original web page appears to have disappeared, copy here:
     * https://fenix.tecnico.ulisboa.pt/downloadFile/845043405449073/Tangent%20Space%20Calculation.pdf
     */

    std::vector<glm::vec3> tangents(m_vertices.size(), glm::vec3(0.0));
    std::vector<glm::vec3> bitangents(m_vertices.size(), glm::vec3(0.0));

    for (const SubMeshDesc &subMesh : m_subMeshes) {
        for (size_t i = 0; i < subMesh.indices.size(); i += 3) {
            const uint16_t i0 = subMesh.indices[i + 0];
            const uint16_t i1 = subMesh.indices[i + 1];
            const uint16_t i2 = subMesh.indices[i + 2];

            const glm::vec3 &p0  = m_vertices[i0].position;
            const glm::vec3 &p1  = m_vertices[i1].position;
            const glm::vec3 &p2  = m_vertices[i2].position;

            const glm::vec2 &uv0 = m_vertices[i0].texcoord;
            const glm::vec2 &uv1 = m_vertices[i1].texcoord;
            const glm::vec2 &uv2 = m_vertices[i2].texcoord;

            const float x1 = p1.x - p0.x;
            const float x2 = p2.x - p0.x;
            const float y1 = p1.y - p0.y;
            const float y2 = p2.y - p0.y;
            const float z1 = p1.z - p0.z;
            const float z2 = p2.z - p0.z;

            const float s1 = uv1.x - uv0.x;
            const float s2 = uv2.x - uv0.x;
            const float t1 = uv1.y - uv0.y;
            const float t2 = uv2.y - uv0.y;

            const float r = 1.0f / (s1 * t2 - s2 * t1);

            const glm::vec3 sdir((t2 * x1 - t1 * x2) * r,
                                 (t2 * y1 - t1 * y2) * r,
                                 (t2 * z1 - t1 * z2) * r);
            const glm::vec3 tdir((s1 * x2 - s2 * x1) * r,
                                 (s1 * y2 - s2 * y1) * r,
                                 (s1 * z2 - s2 * z1) * r);

            tangents[i0] += sdir;
            tangents[i1] += sdir;
            tangents[i2] += sdir;

            bitangents[i0] += tdir;
            bitangents[i1] += tdir;
            bitangents[i2] += tdir;
        }
    }

    for (size_t i = 0; i < m_vertices.size(); i++) {
        Vertex &vertex = m_vertices[i];

        const glm::vec3 &n = vertex.normal;
        const glm::vec3 &t = tangents[i];

        /* Gram-Schmidt orthogonalize. */
        const glm::vec3 tangent = glm::normalize(t - n * glm::dot(n, t));

        /* Calculate handedness of the bitanget, stored in the W component of
         * the tangent vector and is used to calculate the bitangent vector
         * without having to store it separately. */
        const float handedness = (glm::dot(glm::cross(n, t), bitangents[i]) < 0.0f) ? -1.0f : 1.0f;

        vertex.tangent = glm::vec4(tangent, handedness);
    }
}
