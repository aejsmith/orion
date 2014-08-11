/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene management.
 */

#include "render/scene.h"

/** Initialize the scene. */
Scene::Scene(World *world) : m_world(world) {}

/** Destroy the scene. */
Scene::~Scene() {}

/** Add an entity to the scene.
 * @param entity	Entity to add.
 * @param transform	Transformation for the entity. */
void Scene::add_entity(SceneEntity *entity, const Transform &transform) {
	entity->scene = this;
	entity->set_transform(transform);
	m_entities.push_back(entity);
}

/** Remove an entity from the scene.
 * @param entity	Entity to remove. */
void Scene::remove_entity(SceneEntity *entity) {
	m_entities.remove(entity);
	entity->scene = nullptr;
}

/** Set the transformation of an entity in the scene.
 * @param entity	Entity to transform.
 * @param transform	New transformation. */
void Scene::transform_entity(SceneEntity *entity, const Transform &transform) {
	/* When we have proper scene management this will have to move stuff
	 * around in the octree or whatever... */
	entity->set_transform(transform);
}

/** Call a function on each entity visible from a view.
 * @param view		View into the scene.
 * @param func		Function to call on visible entities. */
void Scene::visit_visible_entities(const SceneView *view, const std::function<void (SceneEntity *)> &func) {
	// TODO: Frustum culling.
	for(SceneEntity *entity : m_entities)
		func(entity);
}
