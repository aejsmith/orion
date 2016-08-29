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
 */

#pragma once

#include "core/object.h"

#include <algorithm>
#include <list>
#include <vector>

class Component;
class World;

/**
 * Class representing an entity in the world.
 *
 * All entities that exist in the game world are an instance of this class. It
 * defines basic properties, such as position and orientation. The behaviour of
 * an entity is defined by the components attached to it.
 *
 * Entities in the world form a tree. The transformation properties of an entity
 * are defined relative to its parent's transformation. The transformation
 * functions of this class operate on the relative transformation, except where
 * noted.
 */
class Entity : public Object {
public:
    CLASS();

    /** Transformation change flags. */
    enum TransformFlags {
        /** Position of the entity changed. */
        kPositionChanged = (1 << 0),
        /** Orientation of the entity changed. */
        kOrientationChanged = (1 << 1),
        /** Scale of the entity changed. */
        kScaleChanged = (1 << 2),
    };

    /** Type of the entity list. */
    using EntityList = std::list<ObjectPtr<Entity>>;

    /** Type of the component list. */
    using ComponentList = std::list<ObjectPtr<Component>>;

    /**
     * Public properties.
     */

    /** Name of the entity. */
    PROPERTY() std::string name;

    std::string path() const;

    /**
     * Basic functionality.
     */

    void destroy();

    void tick(float dt);

    /** @return             World that the entity belongs to. */
    World *world() const { return m_world; }
    /** @return             Parent of the entity. */
    Entity *parent() const { return m_parent; }

    VPROPERTY(bool, active);

    void setActive(bool active);

    /**
     * Check the entity's active property.
     *
     * Returns whether the entity is currently active. Note that even if this
     * this entity is marked as active, it will not be active unless all parents
     * in the hierarchy are also active.
     *
     * @return              Whether the entity is active.
     */
    bool active() const { return m_active; }

    /**
     * Check whether the entity is really active.
     *
     * Return whether the entity is really active in the world, i.e. the active
     * property is set and all parents in the hierarchy are also active.
     *
     * @return              Whether the entity is really active.
     */
    bool activeInWorld() const { return m_activeInWorld; }

    /**
     * Children.
     */

    Entity *createChild(std::string name);

    template<typename Func> void visitActiveChildren(Func func);

    /** @return             List of all child entities. */
    const EntityList &children() const { return m_children; }

    /**
     * Components.
     */

    template<typename Type, typename ...Args> Type *createComponent(Args &&...args);
    Component *createComponent(const MetaClass &metaClass);
    template<typename Type> Type *findComponent(bool exactClass = false) const;
    Component *findComponent(const MetaClass &metaClass, bool exactClass = false) const;

    template<typename Func> void visitActiveComponents(Func func);

    /** @return             List of all components. */
    const ComponentList &components() const { return m_components; }

    /**
     * Transformation.
     */

    VPROPERTY(glm::vec3, position);
    VPROPERTY(glm::quat, orientation);
    VPROPERTY(glm::vec3, scale);

    void setPosition(const glm::vec3 &pos);
    void translate(const glm::vec3 &vec);
    void setOrientation(const glm::quat &orientation);
    void rotate(float angle, const glm::vec3 &axis);
    void rotate(const glm::quat &rotation);
    void setScale(const glm::vec3 &scale);

    /** @return             Transformation for the entity. */
    const Transform &transform() const { return m_transform; }
    /** @return             Current relative position. */
    const glm::vec3 &position() const { return m_transform.position(); }
    /** @return             Current relative orientation. */
    const glm::quat &orientation() const { return m_transform.orientation(); }
    /** @return             Current relative scale. */
    const glm::vec3 &scale() const { return m_transform.scale(); }
    /** @return             Local-to-world transformation matrix. */
    const Transform &worldTransform() const { return m_worldTransform; }
    /** @return             Current absolute position. */
    const glm::vec3 &worldPosition() const { return m_worldTransform.position(); }
    /** @return             Current absolute orientation. */
    const glm::quat &worldOrientation() const { return m_worldTransform.orientation(); }
    /** @return             Current absolute scale. */
    const glm::vec3 &worldScale() const { return m_worldTransform.scale(); }
protected:
    ~Entity();

    void serialise(Serialiser &serialiser) const override;
    void deserialise(Serialiser &serialiser) override;
private:
    Entity();

    void addChild(Entity *entity);

    void addComponent(Component *component);
    void removeComponent(Component *component);

    void transformed(unsigned changed);
    void activated();
    void deactivated();

    World *m_world;                         /**< World that this entity belongs to. */
    ObjectPtr<Entity> m_parent;             /**< Parent entity. */
    EntityList m_children;                  /**< Child entities. */
    ComponentList m_components;             /**< Components attached to the entity. */
    bool m_active;                          /**< Whether the entity is active. */

    /**
     * Whether the entity is really active in the world.
     *
     * Whether the active property is set and all parent entities in the
     * hierarchy are active.
     */
    bool m_activeInWorld;

    Transform m_transform;                  /**< Transformation relative to the parent. */

    /**
     * Pre-calculated world transformation.
     *
     * We pre-calcluate the world transformation based on our parent to
     * save having to recalculate it every time they're needed.
     */
    Transform m_worldTransform;

    /** Component needs to use removeComponent(). */
    friend class Component;

    /** World needs access to constructor to create root entity. */
    friend class World;
};

/** Type of a reference-counted pointer to an Entity. */
using EntityPtr = ObjectPtr<Entity>;

/** Create a new component and attach it to the entity.
 * @tparam Type         Type of the component to create.
 * @param args          Arguments to forward to Type's constructor.
 * @return              Pointer to created component. */
template<typename Type, typename ...Args>
Type *Entity::createComponent(Args &&...args) {
    Type *component = new Type(std::forward<Args>(args)...);
    addComponent(component);
    return component;
}

/**
 * Find a component by class.
 *
 * Finds the first component that is an instance of the given class, or of a
 * derived class if exactClass is false (the default).
 *
 * @tparam Type         Class of component to find.
 * @param exactClass    Whether only the exact class (not derived classes)
 *                      should be returned.
 *
 * @return              Pointer to component if found, null if not.
 */
template <typename Type>
inline Type *Entity::findComponent(bool exactClass) const {
    static_assert(
        std::is_base_of<Component, Type>::value,
        "Type must be derived from Component");

    return static_cast<Type *>(findComponent(Type::staticMetaClass));
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
