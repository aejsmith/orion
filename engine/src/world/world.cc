/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		World class.
 */

#include "world/world.h"

/** Currently active world. */
World *g_world = nullptr;

/** Initialize the world. */
World::World() : m_root("root", this) {}

/** Destroy the world. */
World::~World() {
	orion_check(g_world != this, "Destroying active world");
}
