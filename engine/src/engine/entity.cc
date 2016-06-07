/*
 * Copyright (C) 2015-2016 Alex Smith
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
 * @brief               World entity class.
 *
 * TODO:
 *  - Lookup function for entities based on hierarchy (use a path). Also a
 *    lookup function on World that forwards to root entity.
 *  - Disallow transformation of root entity.
 */

#include "engine/component.h"
#include "engine/entity.h"

/** Initialize a new entity.
 * @param name          Name of the entity.
 * @param world         World the entity belongs to. */
Entity::Entity(const std::string &name, World *world) :
    m_name(name),
    m_world(world),
    m_parent(nullptr),
    m_active(false),
    m_activeInWorld(false)
{}

/** Private destructor. To destroy an entity use destroy(). */
Entity::~Entity() {
    /*
     * An entity is deleted when its reference count becomes 0. This should
     * only happen if we have called destroy() to remove references to the
     * entity from the world.
     */
    checkMsg(
        !m_active && m_components.empty() && m_children.empty() && !m_parent,
        "Entity '%s' has no remaining references yet has not been destroyed",
        m_name.c_str());
}

/**
 * Destroy the entity.
 *
 * Destroys the entity. This first deactivates the entity if it is active. Then,
 * all child entities are destroyed, followed by all attached components.
 * Finally the entity is removed from its parent. Once all other remaining
 * references to the entity are released, it will be deleted.
 */
void Entity::destroy() {
    setActive(false);

    while (!m_children.empty()) {
        /* The child's destroy() function removes it from the list. */
        m_children.front()->destroy();
    }

    while (!m_components.empty()) {
        /* Same as above. */
        m_components.front()->destroy();
    }

    if (m_parent) {
        /*
         * Must fetch the parent pointer and set it null before calling remove().
         * It may be that the parent's reference is the last reference and
         * therefore call could result in this entity being deleted.
         */
        EntityPtr parent(std::move(m_parent));
        parent->m_children.remove(this);
    }
}

/** Call the specified function on all components.
 * @param func          Function to call. */
template <typename Func>
inline void Entity::visitComponents(Func func) {
    for (Component *component : m_components)
        func(component);
}

/** Call the specified function on all active components.
 * @param func          Function to call. */
template <typename Func>
inline void Entity::visitActiveComponents(Func func) {
    for (Component *component : m_components) {
        if (component->active())
            func(component);
    }
}

/**
 * Set whether the entity is active.
 *
 * Sets the entity's active property. Note that when setting to true, the entity
 * will not actually become active unless all of its parents in the entity
 * hierarchy are also active.
 *
 * @param active        Whether the entity is active.
 */
void Entity::setActive(bool active) {
    m_active = active;
    if (m_active) {
        if ((!m_parent || m_parent->m_activeInWorld) && !m_activeInWorld)
            activated();
    } else {
        if (m_activeInWorld)
            deactivated();
    }
}

/**
 * Create a child entity.
 *
 * Create a new entity as a child of this entity. The new entity will initially
 * be inactive, and have a relative position of (0, 0, 0) and no relative
 * rotation.
 *
 * @param name          Name of entity to create.
 *
 * @return              Pointer to created entity.
 */
Entity *Entity::createChild(const std::string &name) {
    Entity *entity = new Entity(name, m_world);

    entity->m_parent = this;
    m_children.push_back(entity);

    /* Update the cached transform to incorporate our transformation. */
    entity->transformed(kPositionChanged | kOrientationChanged | kScaleChanged);

    return entity;
}

/**
 * Find a component by class.
 *
 * Finds the first component that is an instance of the given class, or of a
 * derived class if exactClass is false (the default).
 *
 * @param metaClass     Class of component to find.
 * @param exactClass    Whether only the exact class (not derived classes)
 *                      should be returned.
 *
 * @return              Pointer to component if found, null if not.
 */
Component *Entity::findComponent(const MetaClass &metaClass, bool exactClass) const {
    for (Component *component : m_components) {
        if (exactClass) {
            if (&metaClass == &component->metaClass())
                return component;
        } else {
            if (metaClass.isBaseOf(component->metaClass()))
                return component;
        }
    }

    return nullptr;
}

/** Add a component to the entity (internal method).
 * @param component     Component to add. */
void Entity::addComponent(Component *component) {
    /*
     * This only checks for an exact match on class type, so for instance we
     * don't forbid multiple Behaviour-derived classes on the same object.
     */
    checkMsg(
        !findComponent(component->metaClass(), true),
        "Component of type '%s' already exists on entity '%s'",
        component->metaClass().name(), m_name.c_str());

    component->m_entity = this;
    m_components.push_back(component);

    /*
     * We do not need to activate the component at this point as the component
     * is initially inactive. We do however need to let it do anything it needs
     * to with the new transformation.
     */
    component->transformed(kPositionChanged | kOrientationChanged | kScaleChanged);
}

/** Remove a component from the entity (internal method).
 * @param component     Component to remove. */
void Entity::removeComponent(Component *component) {
    /*
     * This avoids an unnecessary reference to the component compared to calling
     * list::remove(), passing the raw pointer to that converts to a reference.
     */
    for (auto it = m_components.begin(); it != m_components.end(); ++it) {
        if (component == *it) {
            m_components.erase(it);
            return;
        }
    }

    checkMsg(
        false, "Removing component '%s' which is not registered on entity '%s'",
        component->metaClass().name(), m_name.c_str());
}

/**
 * Set the position of the entity.
 *
 * Sets the position of the entity. Entity positions are relative to the parent
 * entity's position, so if there were entities A --> B --> C, C's world
 * position would be (A.pos + B.pos + C.pos).
 *
 * @param pos           New position relative to parent.
 */
void Entity::setPosition(const glm::vec3 &pos) {
    m_transform.setPosition(pos);
    transformed(kPositionChanged);
}

/** Translate the position of the entity.
 * @param vec           Vector to move by. */
void Entity::translate(const glm::vec3 &vec) {
    m_transform.setPosition(m_transform.position() + vec);
    transformed(kPositionChanged);
}

/**
 * Set the orientation of the entity.
 *
 * Sets the orientation of the entity. Entity orientations are relative to the
 * parent entity's orientation, so if there were entities A --> B --> C, C's
 * world orientation would be (A.or * B.or * C.or).
 *
 * @param pos           New position relative to parent.
 */
void Entity::setOrientation(const glm::quat &orientation) {
    m_transform.setOrientation(orientation);
    transformed(kOrientationChanged);
}

/** Rotate the entity relative to its current orientation.
 * @param angle         Angle to rotate by (in degrees).
 * @param axis          Axis to rotate around. */
void Entity::rotate(float angle, const glm::vec3 &axis) {
    rotate(glm::angleAxis(glm::radians(angle), glm::normalize(axis)));
}

/** Rotate the entity relative to its current orientation.
 * @param rotation      Quaternion representing rotation to apply. */
void Entity::rotate(const glm::quat &rotation) {
    /* The order of this is important, quaternion multiplication is not
     * commutative. */
    m_transform.setOrientation(rotation * m_transform.orientation());
    transformed(kOrientationChanged);
}

/**
 * Set the scale of the entity.
 *
 * Sets the scale of the entity. Entity scales are relative to the parent
 * entity's scale, so if there were entities A --> B --> C, C's world scale
 * would be (A.scale * B.scale * C.scale).
 *
 * @param scale         New scale relative to parent.
 */
void Entity::setScale(const glm::vec3 &scale) {
    m_transform.setScale(scale);
    transformed(kScaleChanged);
}

/** Update the entity. */
void Entity::tick(float dt) {
    // FIXME: This does not handle activation/deactivation quite well. When
    // an entity becomes active in a frame, it should *not* have it's tick
    // function called in the rest of the frame, otherwise it will get a
    // meaningless dt value. It shouldn't be called until next frame, where
    // dt would be time since activation.

    /* Tick all components. */
    visitActiveComponents([dt](Component *c) { c->tick(dt); });

    /* Tick all children. */
    visitActiveChildren([dt](Entity *e) { e->tick(dt); });
}

/** Called when the transformation has been updated.
 * @param changed       Flags indicating changes made. */
void Entity::transformed(unsigned changed) {
    glm::vec3 position = m_transform.position();
    glm::quat orientation = m_transform.orientation();
    glm::vec3 scale = m_transform.scale();

    /* Recalculate absolute transformations. */
    if (m_parent) {
        glm::vec3 parentPosition = m_parent->worldPosition();
        glm::quat parentOrientation = m_parent->worldOrientation();
        glm::vec3 parentScale = m_parent->worldScale();

        /* Our position must take the parent's orientation and scale into account. */
        position = (parentOrientation * (parentScale * position)) + parentPosition;
        orientation = parentOrientation * orientation;
        scale = parentScale * scale;
    } else {
        checkMsg(
            position == glm::vec3() && orientation == glm::quat() && scale == glm::vec3(),
            "Cannot transform root entity");
    }

    m_worldTransform.set(position, orientation, scale);

    /* Let components know about the transformation. */
    visitComponents([&](Component *c) { c->transformed(changed); });

    /* Visit children and recalculate their transformations. */
    visitChildren([&](Entity *e) { e->transformed(changed); });
}

/** Called when the entity is activated. */
void Entity::activated() {
    m_activeInWorld = true;

    /* Order is important: components on this entity activate before children's
     * components. */
    visitActiveComponents([](Component *c) { c->activated(); });
    visitActiveChildren([](Entity *e) { e->activated(); });
}

/** Called when the entity is deactivated. */
void Entity::deactivated() {
    m_activeInWorld = false;

    /*
     * Order is important: components on children deactivate before this
     * entity's component.
     */
    visitActiveChildren([](Entity *e) { e->deactivated(); });
    visitActiveComponents([](Component *c) { c->deactivated(); });
}
