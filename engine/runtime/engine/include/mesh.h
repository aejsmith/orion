/*
 * Copyright (C) 2015-2017 Alex Smith
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
 * @brief               Mesh asset class.
 */

#pragma once

#include "engine/asset.h"

#include "gpu/index_data.h"
#include "gpu/vertex_data.h"

#include <map>

class Mesh;

/** Sub-component of a Mesh. */
class SubMesh {
public:
    /** @return             Parent mesh. */
    Mesh &parent() const { return m_parent; }
public:
    /** @return             Number of indices in the sub-mesh. */
    size_t numIndices() const { return m_indices->count(); }
    /** @return             Current index data for the sub-mesh. */
    GPUIndexData *indices() const { return m_indices; }

    void setIndices(GPUIndexDataPtr indices);
    void setIndices(const std::vector<uint16_t> &indices);
    void setIndices(const std::vector<uint32_t> &indices);

    size_t material;                        /**< Material index in parent mesh. */
    BoundingBox boundingBox;                /**< Axis-aligned bounding box. */
private:
    explicit SubMesh(Mesh &parent) :
        material (0),
        m_parent (parent)
    {}

    ~SubMesh() {}
private:
    Mesh &m_parent;                         /**< Parent mesh. */
    GPUIndexDataPtr m_indices;              /**< Indices into vertex data. */

    friend class Mesh;
};

/**
 * Mesh asset.
 *
 * This class stores a 3D mesh for rendering. A Mesh is comprised of one or
 * more SubMeshes. This allows different materials to be used on different
 * parts of a mesh.
 */
class Mesh : public Asset {
public:
    CLASS();

    /** Type of the material map. */
    using MaterialMap = std::map<std::string, size_t>;

    Mesh();

    /** @return             Number of submeshes. */
    size_t numSubMeshes() const { return m_children.size(); }
    /** @return             Number of materials. */
    size_t numMaterials() const { return m_materials.size(); }

    /** Get a child at the specified index.
     * @param index         Index of child. Not bounds checked.
     * @return              Child at the specified index. */
    SubMesh &subMesh(size_t index) { return *m_children[index]; }
    const SubMesh &subMesh(size_t index) const { return *m_children[index]; }

    /** @return             Map of material names to indices. */
    const MaterialMap &materials() const { return m_materials; }

    bool material(const std::string &name, size_t &index) const;

    SubMesh &addSubMesh();
    size_t addMaterial(const std::string &name);

    /** @return             Number of vertices in the mesh. */
    size_t numVertices() const { return m_numVertices; }

    GPUVertexData *vertices();

    void setVertices(GPUVertexDataPtr data);

    void setNumVertices(size_t count);

    bool hasAttribute(VertexAttribute::Semantic semantic,
                      unsigned index) const;
    void addAttribute(VertexAttribute::Semantic semantic,
                      unsigned index,
                      VertexAttribute::Type type,
                      bool normalised,
                      size_t components);
    void addAttribute(VertexAttribute::Semantic semantic,
                      unsigned index,
                      VertexAttribute::Type type,
                      bool normalised,
                      size_t components,
                      const void *data,
                      size_t stride);
    void setAttribute(VertexAttribute::Semantic semantic,
                      unsigned index,
                      VertexAttribute::Type type,
                      size_t components,
                      const void *data,
                      size_t stride);

    template <typename T>
    void setAttribute(VertexAttribute::Semantic semantic,
                      unsigned index,
                      const std::vector<T> &data);
protected:
    ~Mesh();
private:
    std::vector<SubMesh *> m_children;      /**< Child submeshes. */

    /**
     * Map of material names.
     *
     * We store an array of known materials with a name, to allow materials
     * to be set on a mesh renderer by name. SubMeshes specify a material
     * index, which references and a table of the materials to use in the
     * mesh renderer.
     */
    MaterialMap m_materials;

    /**
     * Current vertex data.
     *
     * Current GPU vertex data object for the mesh. This is invalidated by
     * setNumVertices() and addAttribute(), and will be recreated on-demand
     * upon a call to vertices() if necessary.
     */
    GPUVertexDataPtr m_vertices;

    size_t m_numVertices;                   /**< Number of vertices. */
    GPUVertexDataLayoutDesc m_layoutDesc;   /**< Layout descriptor. */
    GPUBufferArray m_buffers;               /**< Array of buffers containing mesh data. */
};

/** Type of a mesh pointer. */
using MeshPtr = TypedAssetPtr<Mesh>;

/**
 * Update the data for an attribute from an array.
 *
 * Given an array of data, updates the GPU-side data for the specified attribute
 * from that array. The specified attribute must be present, the type of the
 * data must match that of the attribute, and the size of the array must match
 * the number of vertices in the mesh.
 *
 * @param semantic      Semantic of the attribute to update.
 * @param index         Index of the attribute to update.
 * @param data          Data to update from.
 */
template <typename T>
inline void Mesh::setAttribute(VertexAttribute::Semantic semantic,
                               unsigned index,
                               const std::vector<T> &data)
{
    check(data.size() == numVertices());

    setAttribute(semantic,
                 index,
                 VertexAttributeTypeTraits<T>::kType,
                 VertexAttributeTypeTraits<T>::kComponents,
                 &data[0],
                 sizeof(data[0]));
}
