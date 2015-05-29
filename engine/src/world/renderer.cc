/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Renderer base component.
 */

#include "render/scene.h"
#include "render/scene_entity.h"

#include "world/renderer.h"
#include "world/world.h"

/** Initialise the component.
 * @param entity        Entity that the component belongs to. */
RendererComponent::RendererComponent(Entity *entity) :
    Component(Component::kRendererType, entity)
{}

/** Destory the component. */
RendererComponent::~RendererComponent() {
    /* At this point the entities are not added to the renderer, so just
     * delete them all. */
    while (!m_sceneEntities.empty()) {
        SceneEntity *sceneEntity = m_sceneEntities.front();
        m_sceneEntities.pop_front();
        delete sceneEntity;
    }
}

/** Called when the entity's transformation is updated. */
void RendererComponent::transformed() {
    /* Update all scene entity transformations. */
    if (activeInWorld()) {
        for (SceneEntity *sceneEntity : m_sceneEntities)
            world()->scene()->transformEntity(sceneEntity, worldTransform());
    }
}

/** Called when the component becomes active in the world. */
void RendererComponent::activated() {
    /* Scene entities should not yet be created. Create them. */
    check(m_sceneEntities.empty());
    createSceneEntities(m_sceneEntities);
    check(!m_sceneEntities.empty());

    /* Add them all to the renderer. */
    for (SceneEntity *sceneEntity : m_sceneEntities)
        world()->scene()->addEntity(sceneEntity, worldTransform());
}

/** Called when the component becomes inactive in the world. */
void RendererComponent::deactivated() {
    while (!m_sceneEntities.empty()) {
        SceneEntity *sceneEntity = m_sceneEntities.back();
        m_sceneEntities.pop_back();
        world()->scene()->removeEntity(sceneEntity);
    }
}
