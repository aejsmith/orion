/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Physics material class.
 */

#include "physics_priv.h"

#include "physics/physics_material.h"

/**
 * Initialise the physics material to default values.
 *
 * Initialises the physics material to default values of 0.5 for the friction
 * coefficient and 0.6 for the restitution coefficient.
 */
PhysicsMaterial::PhysicsMaterial() :
    m_restitution(0.6f),
    m_friction(0.5f)
{}

/** Destroy the physics material. */
PhysicsMaterial::~PhysicsMaterial() {}

/** Set the restitution (bounciness) coefficient.
 * @param restitution   New restitution coefficient. */
void PhysicsMaterial::setRestitution(float restitution) {
    check(restitution >= 0.0f && restitution <= 1.0f);

    m_restitution = restitution;
}

/** Set the friction coefficient.
 * @param friction      New friction coefficient. */
void PhysicsMaterial::setFriction(float friction) {
    check(friction >= 0.0f);

    m_friction = friction;
}
