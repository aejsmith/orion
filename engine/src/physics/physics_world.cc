/**
 * @file
 * @copyright           2015 Alex Smith
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
