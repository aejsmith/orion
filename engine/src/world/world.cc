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
	/* Create the renderer's scene manager for the world. */
	m_scene = new Scene(this);
}

/** Destroy the world. */
World::~World() {
	delete m_scene;
}
