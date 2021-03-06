/*
 * Copyright (C) 2015-2016 Alex Smith
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

#include "engine/serialiser.h"
#include "engine/world.h"

/** Initialize the world system. */
WorldSystem::WorldSystem() :
    m_world(nullptr)
{}

/** Destroy the world system. */
WorldSystem::~WorldSystem() {}

/** Initialize the world. */
World::World() {
    /* Create the root entity. */
    m_root = new Entity();
    m_root->name = "root";
    m_root->m_world = this;
    m_root->setActive(true);
}

/** Destroy the world. */
World::~World() {
    m_root->destroy();
}

/** Serialise the world.
 * @param serialiser    Serialiser to write to. */
void World::serialise(Serialiser &serialiser) const {
    Asset::serialise(serialiser);
    serialiser.write("root", m_root);
}

/** Deserialise the world.
 * @param serialiser    Serialiser to write to. */
void World::deserialise(Serialiser &serialiser) {
    Asset::deserialise(serialiser);

    /* Deserialise all entities. */
    EntityPtr newRoot;
    if (serialiser.read("root", newRoot)) {
        /* Destroy existing root to make sure it is safe to free. */
        m_root->destroy();

        m_root = std::move(newRoot);

        /* Entity::deserialise() does not activate the root entity. Do this now. */
        m_root->name = "root";
        m_root->setActive(true);
    }
}

/** Update the world.
 * @param dt            Time elapsed since last update in seconds. */
void World::tick(float dt) {
    /* Update all systems. */
    for (const auto &it : m_systems)
        it.second->tick(dt);

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

/**
 * Get a world system.
 *
 * Gets a global per-world system for this world. If the world doesn't yet have
 * the specified system, it will be created.
 *
 * @param metaClass     Meta-class of of the system.
 *
 * @return              Reference to the world system.
 */
WorldSystem &World::getSystem(const MetaClass &metaClass) {
    ReferencePtr<WorldSystem> &system = m_systems[&metaClass];

    if (!system) {
        system = metaClass.construct().staticCast<WorldSystem>();
        system->m_world = this;
        system->init();
    }

    return *system;
}
