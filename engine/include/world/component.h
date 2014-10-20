/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Component class.
 */

#pragma once

#include "world/entity.h"

class Entity;
class World;

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
 *    |-> destructors
 *
 * As can be seen, this ensures that the hook functions are called when the
 * component is fully constructed.
 */
class Component : Noncopyable {
public:
	/** Component type IDs. */
	enum Type {
		kBehaviourType,
		kCameraType,
		kColliderType,
		kLightType,
		kRendererType,
		kRigidBodyType,
		kNumComponentTypes,
	};
public:
	void destroy();
	void setActive(bool active);

	/** @return		Type ID of the component. */
	Type type() const { return m_type; }
	/** @return		Entity that the component is attached to. */
	Entity *entity() const { return m_entity; }
	/** @return		Whether the component is active. */
	bool active() const { return m_active; }

	bool activeInWorld() const;

	/**
	 * Entity property shortcut functions.
	 */

	/** @return		World that the entity belongs to. */
	World *world() const { return m_entity->world(); }
	/** @return		Transformation for the entity. */
	const Transform &transform() const { return m_entity->transform(); }
	/** @return		Entity relative position. */
	const glm::vec3 &position() const { return m_entity->position(); }
	/** @return		Entity relative orientation. */
	const glm::quat &orientation() const { return m_entity->orientation(); }
	/** @return		Entity relative scale. */
	const glm::vec3 &scale() const { return m_entity->scale(); }
	/** @return		Entity local-to-world transformation matrix. */
	const Transform &worldTransform() const { return m_entity->worldTransform(); }
	/** @return		Entity absolute position. */
	const glm::vec3 &worldPosition() const { return m_entity->worldPosition(); }
	/** @return		Entity absolute orientation. */
	const glm::quat &worldOrientation() const { return m_entity->worldOrientation(); }
	/** @return		Entity absolute scale. */
	const glm::vec3 &worldScale() const { return m_entity->worldScale(); }

	/**
	 * Hook functions.
	 */

	/** Called when the entity's transformation is changed. */
	virtual void transformed() {}

	/** Called when the component becomes active in the world. */
	virtual void activated() {}

	/** Called when the component becomes inactive in the world. */
	virtual void deactivated() {}

	/**
	 * Update the component.
	 *
	 * Called every frame while the component is active in the world to
	 * perform per-frame updates. The supplied time delta is the time since
	 * the last call to this function. This function is not called at a
	 * fixed interval, it is dependent on the frame rate. Therefore, the
	 * time delta should be used to make updates independent of the frame
	 * rate.
	 *
	 * @param dt		Time delta since last update in seconds.
	 */
	virtual void tick(float dt) {}
protected:
	Component(Type type, Entity *entity);
	virtual ~Component();
private:
	Type m_type;			/**< Type of the component. */
	Entity *m_entity;		/**< Entity that the component is attached to. */
	bool m_active;			/**< Whether the component is active. */

	friend class Entity;
};

/** Declare a component type. */
#define DECLARE_COMPONENT(type) \
	static const Component::Type kComponentTypeID = (type)
