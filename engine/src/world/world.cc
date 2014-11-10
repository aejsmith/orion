/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               World class.
 */

#include "render/scene.h"

#include "world/world.h"

/** Initialize the world. */
World::World() {
    /* Create the renderer's scene manager for the world. */
    m_scene = new Scene(this);

    /* Create the root entity. */
    m_root = new Entity("root", this);
    m_root->setActive(true);
}

/** Destroy the world. */
World::~World() {
    m_root->destroy();
    delete m_scene;
}

/** Update the world.
 * @param dt            Time elapsed since last update in seconds. */
void World::tick(float dt) {
    /* Update all entities. */
    m_root->tick(dt);
}

/**
 * Create an entity in the world.
 *
 * Create a new entity as a child of the world's root entity. The new entity
 * will initially be inactive, and have a position of (0, 0, 0) and no rotation.
 *
 * @param name          Name of entity to create.
 *
 * @return              Pointer to created entity.
 */
Entity *World::createEntity(const std::string &name) {
    return m_root->createChild(name);
}
