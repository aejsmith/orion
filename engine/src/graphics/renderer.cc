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
 * @brief               Renderer base component.
 */

#include "engine/world.h"

#include "graphics/renderer.h"

#include "render/scene.h"
#include "render/scene_entity.h"

/** Initialise the component.
 * @param entity        Entity that the component belongs to. */
Renderer::Renderer(Entity *entity) :
    Component(Component::kRendererType, entity),
    m_castShadow(true)
{}

/** Destory the component. */
Renderer::~Renderer() {
    check(m_sceneEntities.empty());
}

/** Set whether the rendered object casts a shadow.
 * @param castShadow    Whether to cast a shadow. */
void Renderer::setCastShadow(bool castShadow) {
    if (castShadow != m_castShadow) {
        m_castShadow = castShadow;

        for (SceneEntity *sceneEntity : m_sceneEntities)
            sceneEntity->setCastShadow(castShadow);
    }
}

/** Called when the entity's transformation is updated.
 * @param changed       Flags indicating changes made. */
void Renderer::transformed(unsigned changed) {
    /* Update all scene entity transformations. */
    for (SceneEntity *sceneEntity : m_sceneEntities)
        sceneEntity->setTransform(worldTransform());
}

/** Called when the component becomes active in the world. */
void Renderer::activated() {
    /* Scene entities should not yet be created. Create them. */
    check(m_sceneEntities.empty());
    createSceneEntities(m_sceneEntities);
    check(!m_sceneEntities.empty());

    /* Set properties and add them all to the renderer. */
    for (SceneEntity *sceneEntity : m_sceneEntities) {
        sceneEntity->setTransform(worldTransform());
        sceneEntity->setCastShadow(m_castShadow);

        world()->scene()->addEntity(sceneEntity);
    }
}

/** Called when the component becomes inactive in the world. */
void Renderer::deactivated() {
    while (!m_sceneEntities.empty()) {
        SceneEntity *sceneEntity = m_sceneEntities.back();
        m_sceneEntities.pop_back();

        world()->scene()->removeEntity(sceneEntity);

        delete sceneEntity;
    }
}
