/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		World class.
 */

#ifndef ORION_WORLD_WORLD_H
#define ORION_WORLD_WORLD_H

#include "world/entity.h"

/**
 * Class holding the game world.
 *
 * This class holds the entire game world. It holds a hierarchical view of all
 * entities in the world. Other systems (e.g. the renderer and the physics
 * system) hold their own views of the world in addition to this. Adding
 * entities to these systems is handled automatically when they are added to
 * this class.
 */
class World : Noncopyable {
public:
	World();
	~World();

	/** Get the root entity.
	 * @return		Pointer to root entity. */
	Entity *root() { return &m_root; }
private:
	/** Root of the entity hierarchy. */
	Entity m_root;
};

extern World *g_world;

#endif /* ORION_WORLD_WORLD_H */
