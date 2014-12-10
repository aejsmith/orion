/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               World entity class.
 *
 * @todo                Lookup function for entities based on hierarchy (use
 *                      a path). Also a lookup function on World that forwards
 *                      to root entity.
 * @fixme               Disallow transformation of root entity.
 */

#include "world/component.h"
#include "world/entity.h"

/** Initialize a new entity.
 * @param name          Name of the entity.
 * @param world         World the entity belongs to. */
Entity::Entity(const std::string &name, World *world) :
    m_name(name),
    m_world(world),
    m_parent(nullptr),
    m_components(Component::kNumComponentTypes, nullptr),
    m_active(false),
    m_activeInWorld(false)
{}

/** Private destructor. To destroy an entity use destroy(). */
Entity::~Entity() {}

/**
 * Destroy the entity.
 *
 * Destroys the entity. All attached components and all child entities will
 * also be deleted.
 */
void Entity::destroy() {
    setActive(false);

    while (!m_children.empty()) {
        /* The child's destroy() function removes it from the list. */
        m_children.front()->destroy();
    }

    for (Component *component : m_components) {
        if (component)
            component->destroy();
    }

    if (m_parent)
        m_parent->m_children.remove(this);

    delete this;
}

/** Call the specified function on all children.
 * @param func          Function to call. */
template <typename Func>
inline void Entity::visitChildren(Func func) {
    for (Entity *child : m_children)
        func(child);
}

/** Call the specified function on all active children.
 * @param func          Function to call. */
template <typename Func>
inline void Entity::visitActiveChildren(Func func) {
    for (Entity *child : m_children) {
        if (child->active())
            func(child);
    }
}

/** Call the specified function on all components.
 * @param func          Function to call. */
template <typename Func>
inline void Entity::visitComponents(Func func) {
    for (Component *component : m_components) {
        if (component)
            func(component);
    }
}

/** Call the specified function on all active components.
 * @param func          Function to call. */
template <typename Func>
inline void Entity::visitActiveComponents(Func func) {
    for (Component *component : m_components) {
        if (component && component->active())
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
    entity->transformed();

    return entity;
}

/** Add a component to the entity (internal method).
 * @param component     Component to add. */
void Entity::addComponent(Component *component) {
    checkMsg(!m_components[component->type()], "Component of type %d already registered", component->type());

    component->m_entity = this;
    m_components[component->type()] = component;

    /* We do not need to activate the component at this point as the component
     * is initially inactive. We do however need to let it do anything it needs
     * to with the new transformation. */
    component->transformed();
}

/** Remove a component from the entity (internal method).
 * @param component     Component to remove. */
void Entity::removeComponent(Component *component) {
    check(m_components[component->type()] == component);
    m_components[component->type()] = nullptr;
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
    transformed();
}

/** Translate the position of the entity.
 * @param vec           Vector to move by. */
void Entity::translate(const glm::vec3 &vec) {
    m_transform.setPosition(m_transform.position() + vec);
    transformed();
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
    transformed();
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
    transformed();
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
    transformed();
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

/** Called when the transformation has been updated. */
void Entity::transformed() {
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
    visitComponents([](Component *c) { c->transformed(); });

    /* Visit children and recalculate their transformations. */
    visitChildren([](Entity *e) { e->transformed(); });
}

/** Called when the entity is activated. */
void Entity::activated() {
    m_activeInWorld = true;

    visitActiveComponents([](Component *c) { c->activated(); });
    visitActiveChildren([](Entity *e) { e->activated(); });
}

/** Called when the entity is deactivated. */
void Entity::deactivated() {
    m_activeInWorld = false;

    visitActiveChildren([](Entity *e) { e->deactivated(); });
    visitActiveComponents([](Component *c) { c->deactivated(); });
}
