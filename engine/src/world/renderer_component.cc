/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Renderer base component.
 */

#include "render/scene.h"
#include "render/scene_entity.h"

#include "world/renderer_component.h"
#include "world/world.h"

/** Initialise the component.
 * @param entity	Entity that the component belongs to. */
RendererComponent::RendererComponent(Entity *entity) :
	Component(Component::kRendererType, entity)
{}

/** Destory the component. */
RendererComponent::~RendererComponent() {
	/* At this point the entities are not added to the renderer, so just
	 * delete them all. */
	while(!m_sceneEntities.empty()) {
		SceneEntity *sceneEntity = m_sceneEntities.front();
		m_sceneEntities.pop_front();
		delete sceneEntity;
	}
}

/** Called when the entity's transformation is updated. */
void RendererComponent::transformed() {
	/* Update all scene entity transformations. */
	if(activeInWorld()) {
		Scene *scene = entity()->world()->scene();
		for(SceneEntity *sceneEntity : m_sceneEntities)
			scene->transformEntity(sceneEntity, entity()->worldTransform());
	}
}

/** Called when the component becomes active in the world. */
void RendererComponent::activated() {
	/* Create the scene entities if we haven't already. */
	if(m_sceneEntities.empty()) {
		createSceneEntities(m_sceneEntities);
		check(!m_sceneEntities.empty());

		/* Set initial transformations. */
		RendererComponent::transformed();
	}

	/* Add them to the renderer. */
	Scene *scene = entity()->world()->scene();
	for(SceneEntity *sceneEntity : m_sceneEntities)
		scene->addEntity(sceneEntity, entity()->worldTransform());
}

/** Called when the component becomes inactive in the world. */
void RendererComponent::deactivated() {
	Scene *scene = entity()->world()->scene();
	for(SceneEntity *sceneEntity : m_sceneEntities)
		scene->removeEntity(sceneEntity);
}
