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
 *
 * TODO:
 *  - Triangle mesh support.
 *  - Should we share shapes where possible? Bullet recommends doing so.
 */

#include "physics_priv.h"

#include "physics/collision_shape.h"
#include "physics/physics_manager.h"
#include "physics/rigid_body.h"

/** Initialise the collision shape.
 * @param entity        Entity that the component belongs to.
 * @param shape         Bullet shape object. */
CollisionShape::CollisionShape(Entity *entity, btCollisionShape *shape) :
    Component(Component::kCollisionShapeType, entity),
    m_btShape(shape),
    m_rigidBody(nullptr)
{}

/** Destroy the collision shape. */
CollisionShape::~CollisionShape() {}

/** Called when the entity's transformation is changed. */
void CollisionShape::transformed() {
    if (m_rigidBody)
        m_rigidBody->transformShape(this);
}

/** Called when the component becomes active in the world. */
void CollisionShape::activated() {
    /* Look for the RigidBody we should become a part of. */
    Entity *entity = this->entity();
    while (entity) {
        RigidBody *rigidBody = entity->findComponent<RigidBody>();
        if (rigidBody && rigidBody->active()) {
            rigidBody->addShape(this);
            break;
        }

        entity = entity->parent();
    }
}

/** Called when the component becomes inactive in the world. */
void CollisionShape::deactivated() {
    if (m_rigidBody)
        m_rigidBody->removeShape(this);
}

/** Set the shape.
 * @param shape         New Bullet shape. */
void CollisionShape::setShape(btCollisionShape *shape) {
    shape->setUserPointer(this);

    if (m_rigidBody)
        m_rigidBody->updateShape(this, shape);

    m_btShape.reset(shape);
}

/** Get the CollisionShape from a Bullet shape.
 * @param btShape       Bullet shape to get from.
 * @return              Pointer to the CollisionShape. */
CollisionShape *CollisionShape::fromBtShape(btCollisionShape *btShape) {
    return reinterpret_cast<CollisionShape *>(btShape->getUserPointer());
}

/**
 * Collision shape implementations.
 */

/**
 * Initialise the box collision shape.
 *
 * Initialises the box collision shape with half extents of 0.5 in each
 * direction.
 *
 * @param entity        Entity that the component belongs to.
 */
BoxCollisionShape::BoxCollisionShape(Entity *entity) :
    CollisionShape(entity, new btBoxShape(btVector3(0.5f, 0.5f, 0.5f))),
    m_halfExtents(0.5f, 0.5f, 0.5f)
{}

/** Set the half extents of the box.
 * @param halfExtents   New half extents for the box. */
void BoxCollisionShape::setHalfExtents(const glm::vec3 &halfExtents) {
    m_halfExtents = halfExtents;

    btBoxShape *shape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
    setShape(shape);
}

/**
 * Initialise the capsule collision shape.
 *
 * Initialises the capsule collision shape with a radius and half height of 0.5.
 *
 * @param entity        Entity that the component belongs to.
 */
CapsuleCollisionShape::CapsuleCollisionShape(Entity *entity) :
    CollisionShape(entity, new btCapsuleShape(0.5f, 1.0f)),
    m_radius(0.5f),
    m_halfHeight(0.5f)
{}

/** Set the radius of the capsule.
 * @param radius        New radius. */
void CapsuleCollisionShape::setRadius(float radius) {
    m_radius = radius;
    updateShape();
}

/** Set the half height of the capsule.
 * @param halfHeight    New half height. */
void CapsuleCollisionShape::setHalfHeight(float halfHeight) {
    m_halfHeight = halfHeight;
    updateShape();
}

/** Update the Bullet shape geometry. */
void CapsuleCollisionShape::updateShape() {
    btCapsuleShape *shape = new btCapsuleShape(m_radius, m_halfHeight * 2.0f);
    setShape(shape);
}

/**
 * Initialise the sphere collision shape.
 *
 * Initialises the sphere collision shape with a radius of 0.5.
 *
 * @param entity        Entity that the component belongs to.
 */
SphereCollisionShape::SphereCollisionShape(Entity *entity) :
    CollisionShape(entity, new btSphereShape(0.5f)),
    m_radius(0.5f)
{}

/** Set the radius of the sphere.
 * @param radius        New radius. */
void SphereCollisionShape::setRadius(float radius) {
    m_radius = radius;

    btSphereShape *shape = new btSphereShape(m_radius);
    setShape(shape);
}