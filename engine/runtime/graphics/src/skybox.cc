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

#include "core/string.h"

#include "engine/asset_manager.h"

#include "graphics/skybox.h"

#include "render/render_entity.h"

#include "render_core/geometry.h"
#include "render_core/render_resources.h"

/** Renderer entity for rendering a skybox. */
class SkyboxRenderEntity : public RenderEntity {
public:
    SkyboxRenderEntity(Skybox *parent);

    Geometry geometry() const override;
    Material *material() const override;
private:
    Skybox *m_parent;               /**< Parent skybox. */
};

/** Initialize the entity
 * @param parent        Parent skybox. */
SkyboxRenderEntity::SkyboxRenderEntity(Skybox *parent) :
    m_parent (parent)
{
    /* Don't want to cull the skybox. */
    BoundingBox boundingBox(glm::vec3(-FLT_MAX), glm::vec3(FLT_MAX));
    setBoundingBox(boundingBox);

    this->name = String::format("Skybox '%s'", parent->entity()->path().c_str());
}

/** Get the geometry for the entity.
 * @return              Geometry for the entity. */
Geometry SkyboxRenderEntity::geometry() const {
    /* Skybox is rendered as a quad. The transformation is ignored by the
     * shader. */
    return g_renderResources->quadGeometry();
}

/** Get the material for the entity.
 * @return              Material for the entity. */
Material *SkyboxRenderEntity::material() const {
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
    m_material->setValue("skybox", m_texture);
}

/** Create renderer entities.
 * @param entities      List to populate. */
void Skybox::createRenderEntities(RenderEntityList &entities) {
    checkMsg(m_texture, "No texture set for Skybox");
    checkMsg(!entity()->parent(), "Skybox must be attached to root entity");

    SkyboxRenderEntity *RenderEntity = new SkyboxRenderEntity(this);
    entities.push_back(RenderEntity);
}
