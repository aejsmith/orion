/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Physics material class.
 */

#pragma once

#include "engine/asset.h"

/**
 * Physics material.
 *
 * A physics material is used by the physics simulation to define the physical
 * surface properties. To use a physics material, it should be attached to a
 * RigidBody.
 */
class PhysicsMaterial : public Asset {
public:
    PhysicsMaterial();
    ~PhysicsMaterial();

    /** @return             Restitution (bounciness) coefficient. */
    float restitution() const { return m_restitution; }
    /** @return             Friction coefficient. */
    float friction() const { return m_friction; }

    void setRestitution(float restitution);
    void setFriction(float friction);
private:
    float m_restitution;                /**< Restitution (bounciness) coefficient. */
    float m_friction;                   /**< Friction coefficient. */
};

/** Type of a physics material pointer. */
typedef TypedAssetPtr<PhysicsMaterial> PhysicsMaterialPtr;
