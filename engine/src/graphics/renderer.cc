/**
 * @file
 * @copyright           2015 Alex Smith
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
    /* At this point the entities are not added to the renderer, so just
     * delete them all. */
    while (!m_sceneEntities.empty()) {
        SceneEntity *sceneEntity = m_sceneEntities.front();
        m_sceneEntities.pop_front();
        delete sceneEntity;
    }
}

/** Set whether the rendered object casts a shadow.
 * @param castShadow    Whether to cast a shadow. */
void Renderer::setCastShadow(bool castShadow) {
    if (castShadow != m_castShadow) {
        m_castShadow = castShadow;

        if (activeInWorld()) {
            for (SceneEntity *sceneEntity : m_sceneEntities)
                sceneEntity->setCastShadow(castShadow);
        }
    }
}

/** Called when the entity's transformation is updated. */
void Renderer::transformed() {
    /* Update all scene entity transformations. */
    if (activeInWorld()) {
        for (SceneEntity *sceneEntity : m_sceneEntities)
            world()->scene()->transformEntity(sceneEntity, worldTransform());
    }
}

/** Called when the component becomes active in the world. */
void Renderer::activated() {
    /* Scene entities should not yet be created. Create them. */
    check(m_sceneEntities.empty());
    createSceneEntities(m_sceneEntities);
    check(!m_sceneEntities.empty());

    /* Set properties and add them all to the renderer. */
    for (SceneEntity *sceneEntity : m_sceneEntities) {
        sceneEntity->setCastShadow(m_castShadow);

        world()->scene()->addEntity(sceneEntity, worldTransform());
    }
}

/** Called when the component becomes inactive in the world. */
void Renderer::deactivated() {
    while (!m_sceneEntities.empty()) {
        SceneEntity *sceneEntity = m_sceneEntities.back();
        m_sceneEntities.pop_back();
        world()->scene()->removeEntity(sceneEntity);
    }
}
