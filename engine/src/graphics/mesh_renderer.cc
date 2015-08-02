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
 * @brief               Mesh renderer component.
 */

#include "gpu/gpu_manager.h"

#include "graphics/mesh_renderer.h"

#include "render/geometry.h"
#include "render/scene_entity.h"

/** Scene entity for rendering a SubMesh. */
class SubMeshSceneEntity : public SceneEntity {
public:
    /** Initialize the scene entity.
     * @param mesh          Submesh to render.
     * @param parent        Parent mesh renderer. */
    SubMeshSceneEntity(SubMesh *subMesh, MeshRenderer *parent) :
        m_subMesh(subMesh),
        m_parent(parent)
    {}

    void geometry(Geometry &geometry) const override;
    Material *material() const override;
private:
    SubMesh *m_subMesh;             /**< Submesh to render. */
    MeshRenderer *m_parent;         /**< Parent mesh renderer. */
};

/** Get the geometry for the entity.
 * @param geometry      Draw data structure to fill in. */
void SubMeshSceneEntity::geometry(Geometry &geometry) const {
    geometry.vertices = (m_subMesh->vertices)
        ? m_subMesh->vertices
        : m_subMesh->parent()->sharedVertices;
    geometry.indices = m_subMesh->indices;
    geometry.primitiveType = PrimitiveType::kTriangleList;
}

/** Get the material for the entity.
 * @return              Material for the entity. */
Material *SubMeshSceneEntity::material() const {
    return m_parent->m_materials[m_subMesh->material];
}

/** Initialize the mesh renderer.
 * @param entity        Entity the component belongs to.
 * @param mesh          Mesh to render. */
MeshRenderer::MeshRenderer(Entity *entity, Mesh *mesh) :
    Renderer(entity),
    m_mesh(mesh),
    m_materials(mesh->numMaterials())
{}

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

/** Create scene entities.
 * @param entities      List to populate. */
void MeshRenderer::createSceneEntities(SceneEntityList &entities) {
    for (size_t i = 0; i < m_mesh->numSubMeshes(); i++) {
        SubMeshSceneEntity *entity = new SubMeshSceneEntity(m_mesh->subMesh(i), this);
        entities.push_back(entity);
    }
}