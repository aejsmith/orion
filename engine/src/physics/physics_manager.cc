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
 * @brief               Physics manager.
 */

#include "physics_priv.h"

#include "engine/asset_manager.h"

#include "physics/physics_manager.h"

/** Global Bullet instances. */
btCollisionConfiguration *g_btCollisionConfiguration;
btDispatcher *g_btDispatcher;
btBroadphaseInterface *g_btBroadphase;
btConstraintSolver *g_btConstraintSolver;

/** Global physics manager instance. */
PhysicsManager *g_physicsManager;

/** Initialise the physics manager. */
PhysicsManager::PhysicsManager() {
    /* Set up Bullet global classes. */
    g_btCollisionConfiguration = new btDefaultCollisionConfiguration();
    g_btDispatcher = new btCollisionDispatcher(g_btCollisionConfiguration);
    g_btBroadphase = new btDbvtBroadphase();
    g_btConstraintSolver = new btSequentialImpulseConstraintSolver();

    /* Load the default physics material. */
    m_defaultMaterial = g_assetManager->load<PhysicsMaterial>("engine/physics_materials/default");
}

/** Destroy the physics manager. */
PhysicsManager::~PhysicsManager() {
    delete g_btConstraintSolver;
    delete g_btBroadphase;
    delete g_btDispatcher;
    delete g_btCollisionConfiguration;
}
