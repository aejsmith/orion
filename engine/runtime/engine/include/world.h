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

#pragma once

#include "core/hash_table.h"

#include "engine/asset.h"
#include "engine/entity.h"

class World;

/**
 * Base class for a per-world system.
 *
 * There are various systems which need global per-world state, e.g. graphics,
 * physics. This class forms part of an interface between the base World class
 * and these systems, without having to add direct dependencies on them to
 * World. Global systems can be accessed using the getSystem() method on World
 * and Component. The first time a system is requested, it will be created.
 */
class WorldSystem : public Object {
public:
    CLASS();

    /** @return             World that the system is for. */
    World &world() const { return *m_world; }
protected:
    WorldSystem();
    ~WorldSystem();

    /**
     * Initialise the system.
     *
     * Any initialisation which depends on the World (or other systems) should
     * be done here. When the constructor is called the system has not yet been
     * associated with the World, so it cannot be used there.
     */
    virtual void init() {}

    /** Update the system.
     * @param dt            Time since last update. */
    virtual void tick(float dt) {}

    friend class World;
private:
    World *m_world;                 /** World that this system is for. */
};

/**
 * Class holding the game world.
 *
 * This class holds the entire game world. It holds a hierarchical view of all
 * entities in the world. Other systems (e.g. the renderer and the physics
 * system) hold their own views of the world in addition to this. Adding
 * entities to these systems is handled automatically when they are activated
 * in the world.
 */
class World : public Asset {
public:
    CLASS();

    World();

    void tick(float dt);

    /**
     * Entity management.
     */

    Entity *createEntity(const std::string &name);

    /** @return             Root entity of the world. */
    Entity *root() { return m_root; }

    /**
     * System management.
     */

    template <typename Type> Type &getSystem();
    WorldSystem &getSystem(const MetaClass &metaClass);
protected:
    ~World();

    void serialise(Serialiser &serialiser) const override;
    void deserialise(Serialiser &serialiser) override;
private:
    EntityPtr m_root;               /**< Root of the entity hierarchy. */

    /** Hash table of systems. */
    HashMap<const MetaClass *, ObjectPtr<WorldSystem>> m_systems;
};

/**
 * Get a world system.
 *
 * Gets a global per-world system for this world. If the world doesn't yet have
 * the specified system, it will be created.
 *
 * @tparam Type         Type of the system.
 *
 * @return              Reference to the world system.
 */
template <typename Type>
inline Type &World::getSystem() {
    static_assert(std::is_base_of<WorldSystem, Type>::value, "Type must be derived from Component");
    return static_cast<Type &>(getSystem(Type::staticMetaClass));
}
