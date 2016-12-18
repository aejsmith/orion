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
 * @brief               Mesh renderer component.
 */

#include "core/string.h"

#include "engine/serialiser.h"

#include "gpu/gpu_manager.h"

#include "graphics/mesh_renderer.h"

#include "render/render_entity.h"

#include "render_core/geometry.h"

/** Renderer entity for rendering a SubMesh. */
class SubMeshRenderEntity : public RenderEntity {
public:
    SubMeshRenderEntity(Mesh *mesh, size_t index, MeshRenderer *parent);

    Geometry geometry() const override;
    Material *material() const override;
private:
    SubMesh *m_subMesh;             /**< Submesh to render. */
    MeshRenderer *m_parent;         /**< Parent mesh renderer. */
};

/** Initialize the entity.
 * @param mesh          Mesh the entity is for.
 * @param index         Index of the submesh.
 * @param parent        Parent mesh renderer. */
SubMeshRenderEntity::SubMeshRenderEntity(Mesh *mesh, size_t index, MeshRenderer *parent) :
    m_subMesh(mesh->subMesh(index)),
    m_parent(parent)
{
    setBoundingBox(m_subMesh->boundingBox);

    #ifdef ORION_BUILD_DEBUG
        this->name = String::format("MeshRenderer '%s' SubMesh %zu", parent->entity()->path().c_str(), index);
    #endif
}

/** Get the geometry for the entity.
 * @return              Geometry for the entity. */
Geometry SubMeshRenderEntity::geometry() const {
    Geometry geometry;

    geometry.vertices = (m_subMesh->vertices)
        ? m_subMesh->vertices
        : m_subMesh->parent()->sharedVertices;
    geometry.indices = m_subMesh->indices;
    geometry.primitiveType = PrimitiveType::kTriangleList;

    return geometry;
}

/** Get the material for the entity.
 * @return              Material for the entity. */
Material *SubMeshRenderEntity::material() const {
    return m_parent->m_materials[m_subMesh->material];
}

/** Initialize the mesh renderer. */
MeshRenderer::MeshRenderer() {}

/** Serialise the mesh renderer.
 * @param serialiser    Serialiser to write to. */
void MeshRenderer::serialise(Serialiser &serialiser) const {
    Component::serialise(serialiser);

    /* Serialise materials. */
    serialiser.beginGroup("materials");

    for (const auto &materialPair : m_mesh->materials())
        serialiser.write(materialPair.first.c_str(), m_materials[materialPair.second]);

    serialiser.endGroup();
}

/** Deserialise the mesh renderer.
 * @param serialiser    Serialiser to read from. */
void MeshRenderer::deserialise(Serialiser &serialiser) {
    Component::deserialise(serialiser);

    /* Deserialise materials. */
    if (serialiser.beginGroup("materials")) {
        for (const auto &materialPair : m_mesh->materials())
            serialiser.read(materialPair.first.c_str(), m_materials[materialPair.second]);

        serialiser.endGroup();
    }
}

/**
 * Set the mesh used by the renderer.
 *
 * Sets the mesh that will be rendered by the renderer. This will clear all
 * currently set materials, therefore they will need to be reconfigured for the
 * new mesh.
 *
 * @param mesh          Mesh to use.
 */
void MeshRenderer::setMesh(Mesh *mesh) {
    m_mesh = mesh;

    /* Deactivate and reactivate to re-create the renderer entities. */
    bool wasActive = active();
    setActive(false);

    m_materials.clear();
    m_materials.resize(mesh->numMaterials());

    setActive(wasActive);
}

/** Get the material with the specified name.
 * @param name          Name of the material to get.
 * @return              Pointer to material set. */
Material *MeshRenderer::material(const std::string &name) const {
    size_t index = 0;
    bool ret = m_mesh->material(name, index);
    checkMsg(ret, "Material slot '%s' not found", name.c_str());

    return m_materials[index];
}

/** Get the material with the specified index.
 * @param index         Index of the material to get.
 * @return              Pointer to material set. */
Material *MeshRenderer::material(size_t index) const {
    check(index < m_materials.size());
    return m_materials[index];
}

/**
 * Set the material to use for part of this mesh.
 *
 * A mesh has one or more material slots defined which its submeshes refer to
 * to get the material they will be rendered with. This function sets the
 * material in the specified slot so that all submeshes using that slot will
 * take on that material.
 *
 * @param name          Name of the material to set.
 * @param material      Material to use.
 */
void MeshRenderer::setMaterial(const std::string &name, Material *material) {
    size_t index = 0;
    bool ret = m_mesh->material(name, index);
    checkMsg(ret, "Material slot '%s' not found", name.c_str());

    m_materials[index] = material;
}

/** Set the material to use for part of this mesh.
 * @param index         Index of the material to set.
 * @param material      Material to use. */
void MeshRenderer::setMaterial(size_t index, Material *material) {
    check(index < m_materials.size());
    m_materials[index] = material;
}

/** Create render entities.
 * @param entities      List to populate. */
void MeshRenderer::createRenderEntities(RenderEntityList &entities) {
    checkMsg(m_mesh, "No mesh set for MeshRenderer on '%s'", entity()->name.c_str());

    for (size_t i = 0; i < m_mesh->numSubMeshes(); i++) {
        SubMeshRenderEntity *entity = new SubMeshRenderEntity(m_mesh, i, this);
        entities.push_back(entity);
    }
}
