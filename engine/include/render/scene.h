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

	void add_entity(SceneEntity *entity, const Transform &transform);
	void remove_entity(SceneEntity *entity);
	void transform_entity(SceneEntity *entity, const Transform &transform);

	void visit_visible_entities(const SceneView *view, const std::function<void (SceneEntity *)> &func);

	/** @return		World that the scene corresponds to. */
	World *world() const { return m_world; }
private:
	World *m_world;			/**< World that the scene corresponds to. */

	/** List of registered entities. */
	SceneEntityList m_entities;
};

#endif /* ORION_RENDER_SCENE_H */
