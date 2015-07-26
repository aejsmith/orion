/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Physics world class.
 */

#pragma once

#include "core/core.h"

class btDiscreteDynamicsWorld;

/** Physics state for a world. */
class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    void tick(float dt);

    /** @return             Gravity vector. */
    const glm::vec3 &gravity() const { return m_gravity; }

    void setGravity(const glm::vec3 &gravity);
private:
    glm::vec3 m_gravity;            /**< Gravity vector. */

    /** Bullet world. */
    std::unique_ptr<btDiscreteDynamicsWorld> m_btWorld;

    friend class RigidBody;
};
