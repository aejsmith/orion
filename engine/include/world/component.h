/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Component class.
 */

#ifndef ORION_WORLD_COMPONENT_H
#define ORION_WORLD_COMPONENT_H

#include "core/defs.h"

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

	/** Get the type ID of the component.
	 * @return		Type ID of the component. */
	Type type() const { return m_type; }

	/** Get the entity that the component is attached to.
	 * @return		Entity that the component is attached to. */
	Entity *entity() const { return m_entity; }

	/** Get whether the component is active.
	 * @return		Whether the component is active. */
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
};

/** Declare a component type. */
#define ORION_COMPONENT(type) \
	static const Component::Type kComponentTypeID = (type)

#endif /* ORION_WORLD_COMPONENT_H */
