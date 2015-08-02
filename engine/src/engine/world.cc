/*
 * Copyright (C) 2015 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               World class.
 */

#include "engine/world.h"

#include "physics/physics_world.h"

#include "render/scene.h"

/** Initialize the world. */
World::World() {
    /* Create the renderer's scene manager for the world. */
    m_scene = new Scene(this);

    /* Create the physics world. */
    m_physics = new PhysicsWorld;

    /* Create the root entity. */
    m_root = new Entity("root", this);
    m_root->setActive(true);
}

/** Destroy the world. */
World::~World() {
    m_root->destroy();
    delete m_physics;
    delete m_scene;
}

/** Update the world.
 * @param dt            Time elapsed since last update in seconds. */
void World::tick(float dt) {
    /* Update the physics simulation. */
    m_physics->tick(dt);

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