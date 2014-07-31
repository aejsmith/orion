/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Component class.
 */

#ifndef ORION_WORLD_COMPONENT_H
#define ORION_WORLD_COMPONENT_H

#include "core/defs.h"

#include "lib/noncopyable.h"

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
 * Components should always be created through Entity::create_component(). This
 * constructs the component and handles attaching it to the entity. They should
 * only be destroyed by calling destroy(). The function call sequence for
 * creating a component is:
 *
 *   Entity::create_component()
 *    |-> constructors
 *    |-> Entity::add_component()
 *    |-> Component::transformed()
 *
 * The call sequence for destroying a component is:
 *
 *   Component::destroy()
 *    |-> Component::deactivated() (if currently active)
 *    |-> Entity::remove_component()
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
		kLightType,
		kColliderType,
		kMeshRendererType,
		kRigidBodyType,
		kNumComponentTypes,
	};
public:
	void destroy();
	void set_active(bool active);

	/** @return		Type ID of the component. */
	Type type() const { return m_type; }
	/** @return		Entity that the component is attached to. */
	Entity *entity() const { return m_entity; }
	/** @return		Whether the component is active. */
	bool active() const { return m_active; }

	bool active_in_world() const;

	/**
	 * Hook functions.
	 */

	/** Called when the entity's transformation is changed. */
	virtual void transformed() {}

	/** Called when the component becomes active in the world. */
	virtual void activated() {}

	/** Called when the component becomes inactive in the world. */
	virtual void deactivated() {}
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
#define ORION_COMPONENT(type) \
	static const Component::Type kComponentTypeID = (type)

#endif /* ORION_WORLD_COMPONENT_H */
