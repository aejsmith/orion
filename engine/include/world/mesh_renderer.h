/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Mesh renderer component.
 */

#pragma once

#include "engine/material.h"
#include "engine/mesh.h"

#include "world/renderer.h"

/** Component which renders a mesh. */
class MeshRenderer : public RendererComponent {
public:
    MeshRenderer(Entity *entity, Mesh *mesh);

    /** @return             Mesh that this component renders. */
    Mesh *mesh() const { return m_mesh; }

    Material *material(const std::string &name) const;
    Material *material(size_t index) const;
    void setMaterial(const std::string &name, Material *material);
    void setMaterial(size_t index, Material *material);
protected:
    virtual void createSceneEntities(SceneEntityList &entities) override;
private:
    MeshPtr m_mesh;                 /**< Mesh to render. */

    /** Array of materials. */
    std::vector<MaterialPtr> m_materials;

    friend class SubMeshSceneEntity;
};
