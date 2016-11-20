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
 * @brief               Physics world class.
 */

#include "physics_priv.h"

#include "physics/physics_world.h"

/** Initialise the world. */
PhysicsWorld::PhysicsWorld() :
    m_btWorld(new btDiscreteDynamicsWorld(
        g_btDispatcher,
        g_btBroadphase,
        g_btConstraintSolver,
        g_btCollisionConfiguration))
{
    setGravity(glm::vec3(0.0f, -9.81f, 0.0f));
}

/** Destroy the world. */
PhysicsWorld::~PhysicsWorld() {}

/** Update the physics simulation.
 * @param dt            Time since last update. */
void PhysicsWorld::tick(float dt) {
    m_btWorld->stepSimulation(dt, 10);
}

/** Set the gravity of the world.
 * @param gravity       New gravity vector. */
void PhysicsWorld::setGravity(const glm::vec3 &gravity) {
    m_btWorld->setGravity(BulletUtil::toBullet(gravity));
}
