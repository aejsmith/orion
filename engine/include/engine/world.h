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

#pragma once

#include "engine/entity.h"

class PhysicsWorld;
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
class World : public Object {
public:
    CLASS();

    World();

    void tick(float dt);

    /** @return             Renderer's scene manager. */
    Scene *scene() const { return m_scene; }
    /** @return             Physics world. */
    PhysicsWorld *physics() const { return m_physics; }

    /**
     * Entity management.
     */

    Entity *createEntity(const std::string &name);

    /** @return             Root entity of the world. */
    Entity *root() { return m_root; }
protected:
    ~World();
private:
    EntityPtr m_root;               /**< Root of the entity hierarchy. */
    Scene *m_scene;                 /**< Scene manager. */
    PhysicsWorld *m_physics;        /**< Physics world. */
};
