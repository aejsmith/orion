/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Component class.
 */

#include "world/component.h"
#include "world/entity.h"

/**
 * Construct the component.
 *
 * Construct the component and attach it to the specified entity. A component
 * is tied to the entity it is created for; it cannot be moved to another
 * entity.
 *
 * @param type		Component type ID.
 * @param entity	Entity owning the component.
 */
Component::Component(Type type, Entity *entity) :
	m_type(type),
	m_entity(entity),
	m_active(false)
{
	/* Add to the entity. */
	m_entity->add_component(this);
}

/**
 * Private destructor.
 *
 * This destructor is protected, as a component can only be destroyed by
 * calling destroy(). The component will be deactivated by destroy() before
 * the destructors are called.
 */
Component::~Component() {
	/* Remove from the parent. */
	m_entity->remove_component(this);
}

/** Destroy the component. */
void Component::destroy() {
	set_active(false);
	delete this;
}

/**
 * Set whether the component is active.
 *
 * Sets the component's active property. Note that a component is only really
 * active if the entity it is attached to is active in the world.
 *
 * @param active	Whether the component should be active.
 */
void Component::set_active(bool active) {
	bool was_active = active_in_world();

	m_active = active;
	if(m_active) {
		if(!was_active && m_entity->active_in_world())
			activated();
	} else {
		if(was_active)
			deactivated();
	}
}

/**
 * Get whether the component is really active.
 *
 * A component is only active when its active property is set to true and the
 * entity it is attached to is active in the world. This is a convenience
 * function to check both of those.
 *
 * @return		Whether the component is really active.
 */
bool Component::active_in_world() const {
	return (m_active && m_entity->active_in_world());
}
