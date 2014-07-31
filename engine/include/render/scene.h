/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene management.
 */

#ifndef ORION_RENDER_SCENE_H
#define ORION_RENDER_SCENE_H

#include "core/defs.h"

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
private:
	World *m_world;			/**< World that the scene corresponds to. */
};

#endif /* ORION_RENDER_SCENE_H */
