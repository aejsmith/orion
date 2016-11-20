/*
 * Copyright (C) 2015 Alex Smith
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
    Mesh *parent() const { return m_parent; }
public:
    GPUVertexDataPtr vertices;          /**< Local vertex data, overrides parent's vertex data. */
    GPUIndexDataPtr indices;            /**< Indices into vertex data. */
    size_t material;                    /**< Material index in parent mesh. */
    BoundingBox boundingBox;            /**< Axis-aligned bounding box. */
private:
    explicit SubMesh(Mesh *parent) : material(0), m_parent(parent) {}
    ~SubMesh() {}
private:
    Mesh *m_parent;                     /**< Parent mesh. */

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
    SubMesh *subMesh(size_t index) { return m_children[index]; }
    const SubMesh *subMesh(size_t index) const { return m_children[index]; }

    /** @return             Map of material names to indices. */
    const MaterialMap &materials() const { return m_materials; }

    bool material(const std::string &name, size_t &index) const;

    SubMesh *addSubMesh();
    size_t addMaterial(const std::string &name);

    GPUVertexDataPtr sharedVertices;    /**< Vertex data shared by all submeshes. */
protected:
    ~Mesh();
private:
    std::vector<SubMesh *> m_children;  /**< Child submeshes. */

    /**
     * Map of material names.
     *
     * We store an array of known materials with a name, to allow materials
     * to be set on a mesh renderer by name. SubMeshes specify a material
     * index, which references and a table of the materials to use in the
     * mesh renderer.
     */
    MaterialMap m_materials;
};

/** Type of a mesh pointer. */
using MeshPtr = TypedAssetPtr<Mesh>;
