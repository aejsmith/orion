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
void Scene::addEntity(SceneEntity *entity, const Transform &transform) {
	entity->setTransform(transform);
	m_entities.push_back(entity);
}

/** Remove an entity from the scene.
 * @param entity	Entity to remove. */
void Scene::removeEntity(SceneEntity *entity) {
	m_entities.remove(entity);
}

/** Set the transformation of an entity in the scene.
 * @param entity	Entity to transform.
 * @param transform	New transformation. */
void Scene::transformEntity(SceneEntity *entity, const Transform &transform) {
	/* When we have proper scene management this will have to move stuff
	 * around in the octree or whatever... */
	entity->setTransform(transform);

	// TODO: Avoid extra work if position hasn't changed?
}

/** Add an light to the scene.
 * @param light		Light to add.
 * @param transform	Transformation for the light. */
void Scene::addLight(SceneLight *light, const glm::vec3 &position) {
	light->setPosition(position);
	m_lights.push_back(light);
}

/** Remove a light from the scene.
 * @param light		Light to remove. */
void Scene::removeLight(SceneLight *light) {
	m_lights.remove(light);
}

/** Set the transformation of a light in the scene.
 * @param light		Light to transform.
 * @param position	New position. */
void Scene::transformLight(SceneLight *light, const glm::vec3 &position) {
	/* Same as above, proper management of lights. */
	light->setPosition(position);
}

/** Call a function on each entity visible from a view.
 * @param view		View into the scene.
 * @param func		Function to call on visible entities. */
void Scene::visitVisibleEntities(const SceneView *view, const std::function<void (SceneEntity *)> &func) {
	// TODO: Frustum culling.
	for(SceneEntity *entity : m_entities)
		func(entity);
}

/** Call a function on each light affecting a view.
 * @param view		View into the scene.
 * @param func		Function to call on visible lights. */
void Scene::visitVisibleLights(const SceneView *view, const std::function<void (SceneLight *)> &func) {
	// TODO: Light culling. Directional/ambient lights always affect.
	for(SceneLight *light : m_lights) {
		/* Ignore lights that would have no contribution. */
		if(!light->intensity() || !glm::length(light->colour()))
			continue;

		func(light);
	}
}
