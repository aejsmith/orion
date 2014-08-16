/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene management.
 */

#include "render/scene.h"
#include "render/scene_light.h"

/** Initialize the scene. */
Scene::Scene(World *world) : m_world(world) {}

/** Destroy the scene. */
Scene::~Scene() {}

/** Add an entity to the scene.
 * @param entity	Entity to add.
 * @param transform	Transformation for the entity. */
void Scene::add_entity(SceneEntity *entity, const Transform &transform) {
	entity->set_transform(transform);
	m_entities.push_back(entity);
}

/** Remove an entity from the scene.
 * @param entity	Entity to remove. */
void Scene::remove_entity(SceneEntity *entity) {
	m_entities.remove(entity);
}

/** Set the transformation of an entity in the scene.
 * @param entity	Entity to transform.
 * @param transform	New transformation. */
void Scene::transform_entity(SceneEntity *entity, const Transform &transform) {
	/* When we have proper scene management this will have to move stuff
	 * around in the octree or whatever... */
	entity->set_transform(transform);

	// TODO: Avoid extra work if position hasn't changed?
}

/** Add an light to the scene.
 * @param light		Light to add.
 * @param transform	Transformation for the light. */
void Scene::add_light(SceneLight *light, const glm::vec3 &position) {
	light->set_position(position);
	m_lights.push_back(light);
}

/** Remove a light from the scene.
 * @param light		Light to remove. */
void Scene::remove_light(SceneLight *light) {
	m_lights.remove(light);
}

/** Set the transformation of a light in the scene.
 * @param light		Light to transform.
 * @param position	New position. */
void Scene::transform_light(SceneLight *light, const glm::vec3 &position) {
	/* Same as above, proper management of lights. */
	light->set_position(position);
}

/** Call a function on each entity visible from a view.
 * @param view		View into the scene.
 * @param func		Function to call on visible entities. */
void Scene::visit_visible_entities(const SceneView *view, const std::function<void (SceneEntity *)> &func) {
	// TODO: Frustum culling.
	for(SceneEntity *entity : m_entities)
		func(entity);
}

/** Call a function on each light affecting a view.
 * @param view		View into the scene.
 * @param func		Function to call on visible lights. */
void Scene::visit_visible_lights(const SceneView *view, const std::function<void (SceneLight *)> &func) {
	// TODO: Light culling. Directional/ambient lights always affect.
	for(SceneLight *light : m_lights) {
		/* Ignore lights that would have no contribution. */
		if(!light->intensity() || !glm::length(light->colour()))
			continue;

		func(light);
	}
}
