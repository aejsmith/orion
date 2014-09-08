/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		World entity class.
 */

#pragma once

#include "world/component.h"

#include <algorithm>
#include <array>
#include <list>
#include <string>

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
class Entity : Noncopyable {
public:
	void destroy();

	void tick(float dt);

	/** @return		World that the entity belongs to. */
	World *world() const { return m_world; }
	/** @return		Parent of the entity. */
	Entity *parent() const { return m_parent; }

	void setActive(bool active);

	/**
	 * Check the entity's active property.
	 *
	 * Returns whether the entity is currently active. Note that even if
	 * this entity is marked as active, it will not be active unless all
	 * parents in the hierarchy are also active.
	 *
	 * @return		Whether the entity is active.
	 */
	bool active() const { return m_active; }

	/**
	 * Check whether the entity is really active.
	 *
	 * Return whether the entity is really active in the world, i.e. the
	 * active property is set and all parents in the hierarchy are also
	 * active.
	 *
	 * @return		Whether the entity is really active.
	 */
	bool activeInWorld() const { return m_activeInWorld; }

	/**
	 * Children.
	 */

	Entity *createChild(const std::string &name);

	/**
	 * Components.
	 */

	template<typename Type, typename ...Args> Type *createComponent(Args &&...args);
	template<typename Type> Type *findComponent() const;

	/**
	 * Transformation.
	 */

	void setPosition(const glm::vec3 &pos);
	void translate(const glm::vec3 &vec);
	void setOrientation(const glm::quat &orientation);
	void rotate(float angle, const glm::vec3 &axis);
	void rotate(const glm::quat &rotation);
	void setScale(const glm::vec3 &scale);

	/** @return		Transformation for the object. */
	const Transform &transform() const { return m_transform; }
	/** @return		Current relative position. */
	const glm::vec3 &position() const { return m_transform.position(); }
	/** @return		Current relative orientation. */
	const glm::quat &orientation() const { return m_transform.orientation(); }
	/** @return		Current relative scale. */
	const glm::vec3 &scale() const { return m_transform.scale(); }
	/** @return		Transformation matrix of the entity. */
	const Transform &worldTransform() const { return m_worldTransform; }
	/** @return		Current absolute position. */
	const glm::vec3 &worldPosition() const { return m_worldTransform.position(); }
	/** @return		Current absolute orientation. */
	const glm::quat &worldOrientation() const { return m_worldTransform.orientation(); }
	/** @return		Current absolute scale. */
	const glm::vec3 &worldScale() const { return m_worldTransform.scale(); }
private:
	/** Type of a list of entities. */
	typedef std::list<Entity *> EntityList;

	/** Type of the component array. */
	typedef std::array<Component *, Component::kNumComponentTypes> ComponentArray;
private:
	Entity(const std::string &name, World *world);
	~Entity();

	void addComponent(Component *component);
	void removeComponent(Component *component);

	template<typename Func> void visitChildren(Func func);
	template<typename Func> void visitActiveChildren(Func func);
	template<typename Func> void visitComponents(Func func);
	template<typename Func> void visitActiveComponents(Func func);

	void transformed();
	void activated();
	void deactivated();
private:
	std::string m_name;		/**< Name of the entity. */
	World *m_world;			/**< World that this entity belongs to. */
	Entity *m_parent;		/**< Parent entity. */
	EntityList m_children;		/**< Child entities. */
	bool m_active;			/**< Whether the entity is active. */

	/**
	 * Whether the entity is really active in the world.
	 *
	 * Whether the active property is set and all parent entities in the
	 * hierarchy are active.
	 */
	bool m_activeInWorld;

	/** Transformation relative to the parent. */
	Transform m_transform;

	/**
	 * Pre-calculated world transformation.
	 *
	 * We pre-calcluate the world transformation based on our parent to
	 * save having to recalculate it every time they're needed.
	 */
	Transform m_worldTransform;

	/** Components attached to the entity. */
	ComponentArray m_components;

	/** Component needs to use removeComponent(). */
	friend class Component;
	/** World needs access to constructor to create root entity. */
	friend class World;
};

/** Create a new component and attach it to the entity.
 * @tparam Type		Type of the component to create.
 * @param args		Arguments to forward to Type's constructor.
 * @return		Pointer to created component. */
template<typename Type, typename ...Args>
Type *Entity::createComponent(Args &&...args) {
	Type *component = new Type(this, std::forward<Args>(args)...);
	addComponent(component);
	return component;
}

/** Get a component attached to the entity.
 * @tparam Type		Type of component to find.
 * @return              Component found, or null if no components of the
 *                      specified type are attached. */
template <typename Type>
inline Type *Entity::findComponent() const {
        return (m_components[Type::kComponentTypeID])
                ? static_cast<Type *>(m_components[Type::kComponentTypeID])
                : nullptr;
}

/** Call the specified function on all children.
 * @param func		Function to call. */
template <typename Func>
inline void Entity::visitChildren(Func func) {
	for(Entity *child : m_children)
		func(child);
}

/** Call the specified function on all active children.
 * @param func		Function to call. */
template <typename Func>
inline void Entity::visitActiveChildren(Func func) {
	for(Entity *child : m_children) {
		if(child->active())
			func(child);
	}
}

/** Call the specified function on all components.
 * @param func		Function to call. */
template <typename Func>
inline void Entity::visitComponents(Func func) {
	for(Component *component : m_components) {
		if(component)
			func(component);
	}
}

/** Call the specified function on all active components.
 * @param func		Function to call. */
template <typename Func>
inline void Entity::visitActiveComponents(Func func) {
	for(Component *component : m_components) {
		if(component && component->active())
			func(component);
	}
}
