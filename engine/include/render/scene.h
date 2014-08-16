/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene management.
 */

#ifndef ORION_RENDER_SCENE_H
#define ORION_RENDER_SCENE_H

#include "core/defs.h"

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

	void add_entity(SceneEntity *entity, const Transform &transform);
	void remove_entity(SceneEntity *entity);
	void transform_entity(SceneEntity *entity, const Transform &transform);

	void add_light(SceneLight *light, const glm::vec3 &position);
	void remove_light(SceneLight *light);
	void transform_light(SceneLight *light, const glm::vec3 &position);

	void visit_visible_entities(const SceneView *view, const std::function<void (SceneEntity *)> &func);
	void visit_visible_lights(const SceneView *view, const std::function<void (SceneLight *)> &func);
private:
	World *m_world;			/**< World that the scene corresponds to. */

	/** List of registered entities. */
	std::list<SceneEntity *> m_entities;

	/** List of registered lights. */
	std::list<SceneLight *> m_lights;
};

#endif /* ORION_RENDER_SCENE_H */
