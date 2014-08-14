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
	while(!m_scene_entities.empty()) {
		SceneEntity *scene_entity = m_scene_entities.front();
		m_scene_entities.pop_front();
		delete scene_entity;
	}
}

/** Called when the entity's transformation is updated. */
void RendererComponent::transformed() {
	/* Update all scene entity transformations. */
	if(active_in_world()) {
		Scene *scene = entity()->world()->scene();
		for(SceneEntity *scene_entity : m_scene_entities)
			scene->transform_entity(scene_entity, entity()->world_transform());
	}
}

/** Called when the component becomes active in the world. */
void RendererComponent::activated() {
	/* Create the scene entities if we haven't already. */
	if(m_scene_entities.empty()) {
		create_scene_entities(m_scene_entities);
		orion_assert(!m_scene_entities.empty());

		/* Set initial transformations. */
		RendererComponent::transformed();
	}

	/* Add them to the renderer. */
	Scene *scene = entity()->world()->scene();
	for(SceneEntity *scene_entity : m_scene_entities)
		scene->add_entity(scene_entity, entity()->world_transform());
}

/** Called when the component becomes inactive in the world. */
void RendererComponent::deactivated() {
	Scene *scene = entity()->world()->scene();
	for(SceneEntity *scene_entity : m_scene_entities)
		scene->remove_entity(scene_entity);
}
