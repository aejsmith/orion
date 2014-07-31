/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		World class.
 */

#include "render/scene.h"

#include "world/world.h"

/** Initialize the world. */
World::World() :
	m_root("root", this)
{
	m_root.set_active(true);

	/* Create the renderer's scene manager for the world. */
	m_scene = new Scene(this);
}

/** Destroy the world. */
World::~World() {
	delete m_scene;
}

/**
 * Create an entity in the world.
 *
 * Create a new entity as a child of the world's root entity. The new entity
 * will initially be inactive, and have a position of (0, 0, 0) and no rotation.
 *
 * @param name		Name of entity to create.
 *
 * @return		Pointer to created entity.
 */
Entity *World::create_entity(const std::string &name) {
	return m_root.create_child(name);
}
