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
 * @brief               Rigid body component.
 */

#pragma once

#include "engine/component.h"

#include "physics/physics_material.h"

class btRigidBody;
class btCollisionShape;
class btCompoundShape;

class CollisionShape;

/**
 * Rigid body component.
 *
 * The rigid body component is used to add an entity to the physics simulation.
 * Rigid bodies must have a shape defined using the CollisionShape component.
 * The body will not truly become active until it also has an active
 * CollisionShape component available.
 *
 * The overall body shape can be defined as a compound of multiple shapes. This
 * is done by creating child entities and attaching CollisionShapes to them.
 * A RigidBody will make use of all CollisionShapes on its own entity and its
 * descendents.
 *
 * Note also that an entity cannot have a RigidBody attached if one is already
 * attached above it in the entity tree.
 */
class RigidBody : public Component {
public:
    DECLARE_COMPONENT(Component::kRigidBodyType);
public:
    explicit RigidBody(Entity *entity);
    ~RigidBody();

    void transformed() override;
    void activated() override;
    void deactivated() override;

    /**
     * Static properties.
     */

    /** @return             Mass of the body. */
    float mass() const { return m_mass; }
    /** @return             Whether the body is static. */
    bool isStatic() const { return m_mass == 0.0f; }
    /** @return             Linear damping factor. */
    float linearDamping() const { return m_linearDamping; }
    /** @return             Angular damping factor. */
    float angularDamping() const { return m_angularDamping; }
    /** @return             Physics material used by the body. */
    PhysicsMaterial *material() const { return m_material; }

    void setMass(float mass);
    void setLinearDamping(float damping);
    void setAngularDamping(float damping);
    void setMaterial(PhysicsMaterial *material);

    /**
     * Dynamic properties updated by the simulation. These can only be used
     * when the body is active.
     */

    glm::vec3 velocity();
    glm::vec3 angularVelocity();

    void setVelocity(const glm::vec3 &velocity);
    void setAngularVelocity(const glm::vec3 &velocity);
private:
    class MotionState;
private:
    void createBody(btCollisionShape *shape);
    void destroyBody();

    btCollisionShape *getShape() const;

    void addShape(CollisionShape *shape);
    void removeShape(CollisionShape *shape);
    void updateShape(CollisionShape *shape, btCollisionShape *btShape);
    void transformShape(CollisionShape *shape);
private:
    float m_mass;                       /**< Mass of the body. */
    float m_linearDamping;              /**< Linear damping factor. */
    float m_angularDamping;             /**< Angular damping factor. */
    PhysicsMaterialPtr m_material;      /**< Physics material. */

    /** Whether a transformation callback from Bullet is in progress. */
    bool m_updatingTransform;

    /** Bullet rigid body. */
    btRigidBody *m_btRigidBody;

    /**
     * Compound shape.
     *
     * When this body has more than one collision shape, they are compiled into
     * a compound shape.
     */
    btCompoundShape *m_btCompoundShape;

    /** Motion state for receiving motion updates from Bullet. */
    std::unique_ptr<MotionState> m_motionState;

    friend class CollisionShape;
};
