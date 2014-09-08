/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene management.
 */

#pragma once

#include "core/core.h"

#include "render/scene_entity.h"

#include <functional>

class SceneLight;
class SceneView;
class World;

/**
 * Renderer's view of the world.
 *
 * The Scene class holds the renderer's view of a world. It only contains the
 * entities which are relevant to the renderer (renderable entities, lights,
 * etc.), and stores them in such a way to allow efficient rendering. The
 * renderer maintains separate views of entities from the world system, which
 * are updated as required by their world counterparts.
 */
class Scene {
public:
	explicit Scene(World *world);
	~Scene();

	/** @return		World that the scene corresponds to. */
	World *world() const { return m_world; }

	void addEntity(SceneEntity *entity, const Transform &transform);
	void removeEntity(SceneEntity *entity);
	void transformEntity(SceneEntity *entity, const Transform &transform);

	void addLight(SceneLight *light, const glm::vec3 &position);
	void removeLight(SceneLight *light);
	void transformLight(SceneLight *light, const glm::vec3 &position);

	void visitVisibleEntities(const SceneView *view, const std::function<void (SceneEntity *)> &func);
	void visitVisibleLights(const SceneView *view, const std::function<void (SceneLight *)> &func);
private:
	World *m_world;			/**< World that the scene corresponds to. */

	/** List of registered entities. */
	std::list<SceneEntity *> m_entities;
	/** List of registered lights. */
	std::list<SceneLight *> m_lights;
};
