/**
 * @file
 * @copyright           2015 Alex Smith
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
EngineGlobal<PhysicsManager> g_physicsManager;

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
