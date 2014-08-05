/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		World entity class.
 *
 * @todo		Lookup function for entities based on hierarchy (use
 *			a path). Also a lookup function on World that forwards
 *			to root entity.
 */

#include "world/entity.h"

/** Initialize a new entity.
 * @param name		Name of the entity.
 * @param world		World the entity belongs to. */
Entity::Entity(const std::string &name, World *world) :
	m_name(name),
	m_world(world),
	m_parent(nullptr),
	m_active(false),
	m_active_in_world(false)
{
	m_components.fill(nullptr);
}

/** Private destructor. To destroy an entity use destroy(). */
Entity::~Entity() {}

/**
 * Destroy the entity.
 *
 * Destroys the entity. All attached components and all child entities will
 * also be deleted.
 */
void Entity::destroy() {
	set_active(false);

	while(!m_children.empty()) {
		/* The child's destroy() function removes it from the list. */
		m_children.front()->destroy();
	}

	for(Component *component : m_components) {
		if(component)
			component->destroy();
	}

	if(m_parent)
		m_parent->m_children.remove(this);

	delete this;
}

/**
 * Set whether the entity is active.
 *
 * Sets the entity's active property. Note that when setting to true, the entity
 * will not actually become active unless all of its parents in the entity
 * hierarchy are also active.
 *
 * @param active	Whether the entity is active.
 */
void Entity::set_active(bool active) {
	m_active = active;
	if(m_active) {
		if((!m_parent || m_parent->m_active_in_world) && !m_active_in_world)
			activated();
	} else {
		if(m_active_in_world)
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
 * @param name		Name of entity to create.
 *
 * @return		Pointer to created entity.
 */
Entity *Entity::create_child(const std::string &name) {
	Entity *entity = new Entity(name, m_world);

	entity->m_parent = this;
	m_children.push_back(entity);

	/* Update the cached transform to incorporate our transformation. */
	entity->transformed();

	return entity;
}

/** Add a component to the entity (internal method).
 * @param component	Component to add. */
void Entity::add_component(Component *component) {
	orion_check(m_parent, "Cannot attach components to root entity");
	orion_check(!m_components[component->type()],
		"Component of type %d already registered",
		component->type());

	component->m_entity = this;
	m_components[component->type()] = component;

	/* We do not need to activate the component at this point as the
	 * component is initially inactive. We do however need to let it do
	 * anything it needs to with the new transformation. */
	component->transformed();
}

/** Remove a component from the entity (internal method).
 * @param component	Component to remove. */
void Entity::remove_component(Component *component) {
	orion_assert(m_components[component->type()] == component);
	m_components[component->type()] = nullptr;
}

/**
 * Set the position of the entity.
 *
 * Sets the position of the entity. Entity positions are relative to the parent
 * entity's position, so if there were entities A --> B --> C, C's world
 * position would be (A.pos + B.pos + C.pos).
 *
 * @param pos		New position relative to parent.
 */
void Entity::set_position(const glm::vec3 &pos) {
	m_transform.set_position(pos);
	transformed();
}

/** Translate the position of the entity.
 * @param vec		Vector to move by. */
void Entity::translate(const glm::vec3 &vec) {
	m_transform.set_position(m_transform.position() + vec);
	transformed();
}

/**
 * Set the orientation of the entity.
 *
 * Sets the orientation of the entity. Entity orientations are relative to the
 * parent entity's orientation, so if there were entities A --> B --> C, C's
 * world orientation would be (A.or * B.or * C.or).
 *
 * @param pos		New position relative to parent.
 */
void Entity::set_orientation(const glm::quat &orientation) {
	m_transform.set_orientation(orientation);
	transformed();
}

/** Rotate the entity relative to its current orientation.
 * @param angle		Angle to rotate by (in degrees).
 * @param axis		Axis to rotate around. */
void Entity::rotate(float angle, const glm::vec3 &axis) {
	rotate(glm::angleAxis(glm::radians(angle), glm::normalize(axis)));
}

/** Rotate the entity relative to its current orientation.
 * @param rotation	Quaternion representing rotation to apply. */
void Entity::rotate(const glm::quat &rotation) {
	/* The order of this is important, quaternion multiplication is not
	 * commutative. */
	m_transform.set_orientation(rotation * m_transform.orientation());
	transformed();
}

/**
 * Set the scale of the entity.
 *
 * Sets the scale of the entity. Entity scales are relative to the parent
 * entity's scale, so if there were entities A --> B --> C, C's world scale
 * would be (A.scale * B.scale * C.scale).
 *
 * @param scale		New scale relative to parent.
 */
void Entity::set_scale(const glm::vec3 &scale) {
	m_transform.set_scale(scale);
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
	visit_active_components([dt](Component *c) { c->tick(dt); });

	/* Tick all children. */
	visit_active_children([dt](Entity *e) { e->tick(dt); });
}

/** Called when the transformation has been updated. */
void Entity::transformed() {
	glm::vec3 position = m_transform.position();
	glm::quat orientation = m_transform.orientation();
	glm::vec3 scale = m_transform.scale();

	/* Recalculate absolute transformations. */
	if(m_parent) {
		glm::vec3 parent_position = m_parent->world_position();
		glm::quat parent_orientation = m_parent->world_orientation();
		glm::vec3 parent_scale = m_parent->world_scale();

		/* Our position must take the parent's orientation and scale
		 * into account. */
		position = (parent_orientation * (parent_scale * position)) + parent_position;
		orientation = parent_orientation * orientation;
		scale = parent_scale * scale;
	}

	m_world_transform.set(position, orientation, scale);

	/* Let components know about the transformation. */
	visit_components([](Component *c) { c->transformed(); });

	/* Visit children and recalculate their transformations. */
	visit_children([](Entity *e) { e->transformed(); });
}

/** Called when the entity is activated. */
void Entity::activated() {
	m_active_in_world = true;

	visit_active_components([](Component *c) { c->activated(); });
	visit_active_children([](Entity *e) { e->activated(); });
}

/** Called when the entity is deactivated. */
void Entity::deactivated() {
	m_active_in_world = false;

	visit_active_children([](Entity *e) { e->deactivated(); });
	visit_active_components([](Component *c) { c->deactivated(); });
}
