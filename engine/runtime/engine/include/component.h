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
 * @brief               Component class.
 */

#pragma once

#include "engine/entity.h"
#include "engine/world.h"

class Component;
class Entity;

/**
 * Class implementing a component.
 *
 * Components implement the behaviour of an entity in the game world. An Entity
 * only provides some minimal properties such as a transformation. All other
 * functionality is implemented in components which are attached to an Entity.
 *
 * Components have a number of hook functions that get called from the Entity
 * to which they are attached, which can be overridden by derived classes to
 * implement their behaviour.
 *
 * Components should always be created through Entity::createComponent(). This
 * constructs the component and handles attaching it to the entity. They should
 * only be destroyed by calling destroy(). The function call sequence for
 * creating a component is:
 *
 *   Entity::createComponent()
 *    |-> constructors
 *    |-> Entity::addComponent()
 *    |-> Component::transformed()
 *
 * The call sequence for destroying a component is:
 *
 *   Component::destroy()
 *    |-> Component::deactivated() (if currently active)
 *    |-> Entity::removeComponent()
 *    |-> destructors (once no other references remain)
 *
 * As can be seen, this ensures that the hook functions are called when the
 * component is fully constructed.
 */
class Component : public Object {
public:
    CLASS();

    void destroy();

    VPROPERTY(bool, active);

    void setActive(bool active);

    /** @return             Entity that the component is attached to. */
    Entity *entity() const { return m_entity; }
    /** @return             Whether the component is active. */
    bool active() const { return m_active; }

    bool activeInWorld() const;

    /**
     * Entity property shortcut functions.
     */

    /** @return             World that the entity belongs to. */
    World *world() const { return m_entity->world(); }

    /**
     * Get a world system.
     *
     * Gets a global per-world system for the world this component belongs to.
     * If the world doesn't yet have the specified system, it will be created.
     *
     * @tparam Type         Type of the system.
     *
     * @return              Reference to the world system.
     */
    template <typename Type> Type &getSystem() { return m_entity->world()->getSystem<Type>(); }

    /** @return             Transformation for the entity. */
    const Transform &transform() const { return m_entity->transform(); }
    /** @return             Entity relative position. */
    const glm::vec3 &position() const { return m_entity->position(); }
    /** @return             Entity relative orientation. */
    const glm::quat &orientation() const { return m_entity->orientation(); }
    /** @return             Entity relative scale. */
    const glm::vec3 &scale() const { return m_entity->scale(); }
    /** @return             Entity local-to-world transformation matrix. */
    const Transform &worldTransform() const { return m_entity->worldTransform(); }
    /** @return             Entity absolute position. */
    const glm::vec3 &worldPosition() const { return m_entity->worldPosition(); }
    /** @return             Entity absolute orientation. */
    const glm::quat &worldOrientation() const { return m_entity->worldOrientation(); }
    /** @return             Entity absolute scale. */
    const glm::vec3 &worldScale() const { return m_entity->worldScale(); }

    /**
     * Hook functions.
     */

    /** Called when the entity's transformation is changed.
     * @param changed       Flags indicating changes made (see
     *                      Entity::TransformFlags). */
    virtual void transformed(unsigned changed) {}

    /** Called when the component becomes active in the world. */
    virtual void activated() {}

    /** Called when the component becomes inactive in the world. */
    virtual void deactivated() {}

    /**
     * Update the component.
     *
     * Called every frame while the component is active in the world to perform
     * per-frame updates. The supplied time delta is the time since the last
     * call to this function. This function is not called at a fixed interval,
     * it is dependent on the frame rate. Therefore, the time delta should be
     * used to make updates independent of the frame rate.
     *
     * @param dt            Time delta since last update in seconds.
     */
    virtual void tick(float dt) {}
protected:
    Component();
    ~Component();

    void serialise(Serialiser &serialiser) const override;
    void deserialise(Serialiser &serialiser) override;
private:
    EntityPtr m_entity;             /**< Entity that the component is attached to. */
    bool m_active;                  /**< Whether the component is active. */

    friend class Entity;
};

/** Type of a pointer to a component. */
using ComponentPtr = ReferencePtr<Component>;

/*
 * Entity template methods which are dependent on Component's definition.
 */

/** Call the specified function on all active components.
 * @param func          Function to call. */
template <typename Func>
inline void Entity::visitActiveComponents(Func func) {
    for (Component *component : m_components) {
        if (component->active())
            func(component);
    }
}
