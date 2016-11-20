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

#pragma once

#include "engine/mesh.h"

#include "graphics/renderer.h"

#include "shader/material.h"

/** Component which renders a mesh. */
class MeshRenderer : public Renderer {
public:
    CLASS();

    MeshRenderer();

    VPROPERTY(MeshPtr, mesh);

    /** @return             Mesh that this component renders. */
    Mesh *mesh() const { return m_mesh; }

    void setMesh(Mesh *mesh);

    Material *material(const std::string &name) const;
    Material *material(size_t index) const;
    void setMaterial(const std::string &name, Material *material);
    void setMaterial(size_t index, Material *material);
protected:
    ~MeshRenderer() {}

    void serialise(Serialiser &serialiser) const override;
    void deserialise(Serialiser &serialiser) override;

    void createSceneEntities(SceneEntityList &entities) override;
private:
    MeshPtr m_mesh;                 /**< Mesh to render. */

    /** Array of materials. */
    std::vector<MaterialPtr> m_materials;

    friend class SubMeshSceneEntity;
};
