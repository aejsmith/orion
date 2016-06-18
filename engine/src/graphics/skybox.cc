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
 * @brief               Skybox component.
 */

#include "engine/asset_manager.h"

#include "graphics/skybox.h"

#include "render/geometry.h"
#include "render/render_manager.h"
#include "render/scene_entity.h"

/** Scene entity for rendering a skybox. */
class SkyboxSceneEntity : public SceneEntity {
public:
    SkyboxSceneEntity(Skybox *parent);

    void geometry(Geometry &geometry) const override;
    Material *material() const override;
private:
    Skybox *m_parent;               /**< Parent skybox. */
};

/** Initialize the skybox scene entity
 * @param parent        Parent skybox. */
SkyboxSceneEntity::SkyboxSceneEntity(Skybox *parent) :
    m_parent(parent)
{
    /* Don't want to cull the skybox. */
    BoundingBox boundingBox(glm::vec3(-FLT_MAX), glm::vec3(FLT_MAX));
    setBoundingBox(boundingBox);
}

/** Get the geometry for the entity.
 * @param geometry      Draw data structure to fill in. */
void SkyboxSceneEntity::geometry(Geometry &geometry) const {
    /* Skybox is rendered as a quad. The transformation is ignored by the
     * shader. */
    g_renderManager->quadGeometry(geometry);
}

/** Get the material for the entity.
 * @return              Material for the entity. */
Material *SkyboxSceneEntity::material() const {
    return m_parent->m_material;
}

/** Initialise the skybox. */
Skybox::Skybox() {
    /* Create the skybox material. */
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/skybox");
    m_material = new Material(shader);
}

/** Set the texture used by the skybox.
 * @param texture       Texture to set. */
void Skybox::setTexture(TextureCube *texture) {
    m_texture = texture;

    // FIXME: Need to make setValue work for different texture types.
    TextureBasePtr baseTexture = texture;
    m_material->setValue("skybox", baseTexture);
}

/** Create scene entities.
 * @param entities      List to populate. */
void Skybox::createSceneEntities(SceneEntityList &entities) {
    checkMsg(m_texture, "No texture set for Skybox");
    checkMsg(!entity()->parent(), "Skybox must be attached to root entity");

    SkyboxSceneEntity *sceneEntity = new SkyboxSceneEntity(this);
    entities.push_back(sceneEntity);
}
