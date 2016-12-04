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
 * @brief               Physics system class.
 */

#include "physics_priv.h"

#include "engine/asset_manager.h"

#include "physics/physics_system.h"

/** Initialise the world. */
PhysicsSystem::PhysicsSystem() {
    /* Create Bullet objects. */
    m_btCollisionConfiguration.reset(new btDefaultCollisionConfiguration());
    m_btDispatcher.reset(new btCollisionDispatcher(m_btCollisionConfiguration.get()));
    m_btBroadphase.reset(new btDbvtBroadphase());
    m_btConstraintSolver.reset(new btSequentialImpulseConstraintSolver());
    m_btWorld.reset(new btDiscreteDynamicsWorld(
        m_btDispatcher.get(),
        m_btBroadphase.get(),
        m_btConstraintSolver.get(),
        m_btCollisionConfiguration.get()));

    setGravity(glm::vec3(0.0f, -9.81f, 0.0f));
}

/** Destroy the world. */
PhysicsSystem::~PhysicsSystem() {}

/** Update the physics simulation.
 * @param dt            Time since last update. */
void PhysicsSystem::tick(float dt) {
    m_btWorld->stepSimulation(dt, 10);
}

/** Set the gravity of the world.
 * @param gravity       New gravity vector. */
void PhysicsSystem::setGravity(const glm::vec3 &gravity) {
    m_btWorld->setGravity(BulletUtil::toBullet(gravity));
}

/** @return             Default physics material. */
PhysicsMaterialPtr PhysicsSystem::defaultMaterial() {
    return g_assetManager->load<PhysicsMaterial>("engine/physics_materials/default");
}
