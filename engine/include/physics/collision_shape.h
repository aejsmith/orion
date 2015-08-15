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
 * @brief               Collision shape component.
 */

#pragma once

#include "engine/component.h"

class btBoxShape;
class btSphereShape;
class btCapsuleShape;
class btCollisionShape;

class RigidBody;

/**
 * Collision shape component.
 *
 * This class defines the shape of an object for physics collision detection
 * purposes. This is a base class for real collision shape types, it cannot
 * be instantiated directly.
 *
 * If a CollisionShape is attached to an entity that does not have a RigidBody
 * attached, and nor do any of its parents, the entity will function as a static
 * collider, i.e. objects can collide with it, but the entity itself will not be
 * affected by physics. Static colliders should not be transformed on a regular
 * basis as doing so is highly inefficient.
 *
 * For an Entity to be fully affected by the physics simulation, it must have
 * a RigidBody attached and at least one CollisionShape attached to it or below
 * it.
 */
class CollisionShape : public Component {
public:
    DECLARE_COMPONENT(Component::kCollisionShapeType);
public:
    ~CollisionShape();

    void transformed(unsigned changed) override;
    void activated() override;
    void deactivated() override;
protected:
    CollisionShape(Entity *entity, btCollisionShape *shape);

    void setShape(btCollisionShape *shape);

    static CollisionShape *fromBtShape(btCollisionShape *btShape);
private:
    /** Bullet collision shape. */
    std::unique_ptr<btCollisionShape> m_btShape;

    /**
     * Pointer to the RigidBody controlling this shape.
     *
     * This does not always belong to the same entity that the shape belongs to.
     * A RigidBody combines all CollisionShapes on its Entity and its children
     * so this points to the body which this shape is a part of. This field is
     * maintained by RigidBody.
     */
    RigidBody *m_rigidBody;

    friend class RigidBody;
};

/**
 * Box collision shape.
 *
 * A box is defined by its half extents, i.e. half of its width, height and
 * depth. The box extends out by those dimensions in both the positive and
 * negative directions on each axis from the entity's local origin.
 */
class BoxCollisionShape : public CollisionShape {
public:
    explicit BoxCollisionShape(Entity *entity);

    /** @return             Half extents of the box. */
    const glm::vec3 &halfExtents() const { return m_halfExtents; }

    void setHalfExtents(const glm::vec3 &halfExtents);
private:
    glm::vec3 m_halfExtents;        /**< Half extents of the box. */
};

/**
 * Capsule collision shape.
 *
 * A capsule is a combination of a cylindrical body and a hemispherical top and
 * bottom. It is defined by the half height of the cylinder, i.e the distance
 * from the entity's local origin to each end of the cylinder, and the radius of
 * the hemispherical ends. Note that with an identity orientation, the capsule
 * is aligned along the X axis.
 */
class CapsuleCollisionShape : public CollisionShape {
public:
    explicit CapsuleCollisionShape(Entity *entity);

    /** @return             Radius of the hemispherical parts of the capsule. */
    float radius() const { return m_radius; }
    /** @return             Half height of the cylindrical part of the capsule. */
    float halfHeight() const { return m_halfHeight; }

    void setRadius(float radius);
    void setHalfHeight(float halfHeight);
private:
    void updateShape();
private:
    float m_radius;                 /**< Radius of the hemispherical part. */
    float m_halfHeight;             /**< Half height of the cylindrical part. */
};

/**
 * Sphere collision shape.
 *
 * A sphere is defined just by its radius, the distance from the entity's local
 * origin to the edge of the sphere.
 */
class SphereCollisionShape : public CollisionShape {
public:
    explicit SphereCollisionShape(Entity *entity);

    /** @return             Radius of the sphere. */
    float radius() const { return m_radius; }

    void setRadius(float radius);
private:
    float m_radius;                 /**< Radius of the sphere. */
};
