/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Component class.
 */

#include "world/component.h"
#include "world/entity.h"

/** Construct the component.
 * @param type		Component type ID.
 * @param entity	Entity the component belongs to. */
Component::Component(Type type, Entity *entity) :
	m_type(type),
	m_entity(entity),
	m_active(false)
{}

/** Private destructor. To destroy a component use destroy(). */
Component::~Component() {}

/** Destroy the component. */
void Component::destroy() {
	set_active(false);

	/* Remove from the parent. */
	m_entity->remove_component(this);

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
