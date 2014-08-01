/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		World class.
 */

#ifndef ORION_WORLD_WORLD_H
#define ORION_WORLD_WORLD_H

#include "world/entity.h"

class Scene;

/**
 * Class holding the game world.
 *
 * This class holds the entire game world. It holds a hierarchical view of all
 * entities in the world. Other systems (e.g. the renderer and the physics
 * system) hold their own views of the world in addition to this. Adding
 * entities to these systems is handled automatically when they are activated
 * in the world.
 */
class World : Noncopyable {
public:
	World();
	~World();

	/** @return		Renderer's scene manager. */
	Scene *scene() const { return m_scene; }

	/**
	 * Entity management.
	 */

	Entity *create_entity(const std::string &name);

	/** @return		Root entity of the world. */
	Entity *root() { return m_root; }
private:
	Entity *m_root;			/**< Root of the entity hierarchy. */
	Scene *m_scene;			/**< Scene manager. */
};

#endif /* ORION_WORLD_WORLD_H */
