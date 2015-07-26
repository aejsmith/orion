/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Physics manager class.
 */

#pragma once

#include "physics/physics_material.h"

/** Global physics system. */
class PhysicsManager : public Noncopyable {
public:
    PhysicsManager();
    ~PhysicsManager();

    /** @return             Default physics material. */
    PhysicsMaterial *defaultMaterial() const { return m_defaultMaterial; }
private:
    /** Default physics material. */
    PhysicsMaterialPtr m_defaultMaterial;
};

extern EngineGlobal<PhysicsManager> g_physicsManager;
