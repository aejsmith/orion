/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		World entity class.
 *
 * @todo		Lookup function for entities based on hierarchy (use
 *			a path). Also a lookup function on World that forwards
 *			to root entity.
 */

#include "gpu/gpu.h"

#include "world/entity.h"

/**
 * Initialize a new entity.
 *
 * Initializes a new entity as a child of the specified entity. The entity will
 * initially be inactive, with a position of (0, 0, 0), no rotation, and a
 * scale of (1, 1, 1). To add an entity at the root of the world, specify the
 * world's root entity obtained from World::root() as the parent.
 *
 * @param name		Name of the entity.
 * @param parent	Parent entity.
 */
Entity::Entity(const std::string &name, Entity *parent) :
	m_name(name),
	m_world(parent->world()),
	m_parent(parent),
	m_active(false),
	m_active_in_world(false),
	m_position(0.0f, 0.0f, 0.0f),
	m_orientation(1.0f, 0.0f, 0.0f, 0.0f),
	m_scale(1.0f),
	m_uniforms_outdated(true)
{
	m_components.fill(nullptr);

	/* Create the uniform buffer. */
	m_uniforms = g_engine->gpu()->create_buffer(
		GPUBuffer::kUniformBuffer,
		GPUBuffer::kDynamicDrawUsage,
		sizeof(EntityUniforms));

	/* Add ourself to the parent. */
	parent->m_children.push_back(this);

	/* Set the cached transform to incorporate the parent's transformation. */
	transformed();
}

/** Private constructor with no parent for root entity initialization.
 * @param name		Name of the entity.
 * @param world		World for the entity. */
Entity::Entity(const std::string &name, World *world) :
	m_name(name),
	m_world(world),
	m_parent(nullptr),
	m_active(true),
	m_active_in_world(true),
	m_position(0.0f, 0.0f, 0.0f),
	m_orientation(1.0f, 0.0f, 0.0f, 0.0f),
	m_scale(1.0f),
	m_world_position(0.0f, 0.0f, 0.0f),
	m_world_orientation(1.0f, 0.0f, 0.0f, 0.0f),
	m_world_scale(1.0f),
	m_uniforms_outdated(false)
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

	while(!m_children.empty())
		m_children.front()->destroy();

	for(Component *component : m_components) {
		if(component)
			component->destroy();
	}

	if(m_parent)
		m_parent->m_children.remove(this);

	delete this;
}

/**
 * Set the parent of the entity.
 *
 * Sets the parent of the entity. The new parent must belong to the same world
 * as the previous parent. If the entity's active property is set but was not
 * actually active due to its parent being inactive, it will become active if
 * the new parent is active. Conversely, if it was active but the new parent is
 * inactive, it will be deactivated.
 *
 * @param parent	New parent.
 */
void Entity::set_parent(Entity *parent) {
	orion_assert(parent);
	orion_check(m_parent, "Cannot set parent of root entity");
	orion_check(parent->m_world == m_parent->m_world, "Cannot set parent to entity in another world");

	/* Deactivate ourself while we're moving. */
	if(m_active_in_world)
		deactivated();

	m_parent->m_children.remove(this);
	m_parent = parent;
	m_parent->m_children.push_back(this);

	/* Update transformation based on new parent. */
	transformed();

	/* Reactivate if we and the new parent are active. */
	if(m_parent->m_active_in_world && m_active)
		activated();
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
		if(m_parent->m_active_in_world && !m_active_in_world)
			activated();
	} else {
		if(m_active_in_world)
			deactivated();
	}
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
void Entity::set_position(glm::vec3 pos) {
	m_position = pos;
	transformed();
}

/** Translate the position of the entity.
 * @param vec		Vector to move by. */
void Entity::translate(glm::vec3 vec) {
	m_position += vec;
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
void Entity::set_orientation(glm::quat orientation) {
	m_orientation = orientation;
	transformed();
}

/** Rotate the entity relative to its current orientation.
 * @param angle		Angle to rotate by (in degrees).
 * @param axis		Axis to rotate around. */
void Entity::rotate(float angle, glm::vec3 axis) {
	axis = glm::normalize(axis);
	rotate(glm::angleAxis(glm::radians(angle), axis));
}

/** Rotate the entity relative to its current orientation.
 * @param rotation	Quaternion representing rotation to apply. */
void Entity::rotate(glm::quat rotation) {
	/* The order of this is important, quaternion multiplication is not
	 * commutative. */
	m_orientation = rotation * m_orientation;
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
void Entity::set_scale(glm::vec3 scale) {
	m_scale = scale;
	transformed();
}

/**
 * Get the uniform buffer containing the entity parameters.
 *
 * Gets the uniform buffer which contains per-entity parameters. This function
 * will update the buffer with the latest parameters if it is currently out of
 * date.
 *
 * @return		Pointer to buffer containing entity parameters.
 */
GPUBufferPtr Entity::uniforms() const {
	if(m_uniforms_outdated) {
		GPUBufferMapper<EntityUniforms> uniforms(
			m_uniforms,
			GPUBuffer::kMapInvalidate,
			GPUBuffer::kWriteAccess);

		memcpy(&uniforms->transform, glm::value_ptr(m_world_transform), sizeof(uniforms->transform));

		m_uniforms_outdated = false;
	}

	return m_uniforms;
}

/** Private function called by Component to register itself.
 * @param component	Component to add. */
void Entity::add_component(Component *component) {
	orion_check(m_parent, "Cannot attach components to root entity");
	orion_check(!m_components[component->type()],
		"Component of type %d already registered",
		component->type());

	/* Do not need to activate the component at this point as we are called
	 * from the Component constructor only, where the component will not
	 * be active. */
	m_components[component->type()] = component;
}

/** Private function called by Component to unregister itself.
 * @param component	Component to remove. */
void Entity::remove_component(Component *component) {
	orion_assert(m_components[component->type()] == component);
	m_components[component->type()] = nullptr;
}

/** Called when the transformation has been updated. */
void Entity::transformed() {
	/* Recalculate absolute transformations. */
	if(m_parent) {
		glm::vec3 parent_position = m_parent->world_position();
		glm::quat parent_orientation = m_parent->world_orientation();
		glm::vec3 parent_scale = m_parent->world_scale();

		/* Our position must take the parent's orientation and scale
		 * into account. */
		m_world_position = (parent_orientation * (parent_scale * m_position)) + parent_position;
		m_world_orientation = parent_orientation * m_orientation;
		m_world_scale = parent_scale * m_scale;
	} else {
		m_world_position = m_position;
		m_world_orientation = m_orientation;
		m_world_scale = m_scale;
	}

	/* Recalculate our transformation matrix. */
	glm::mat4 position = glm::translate(glm::mat4(), m_world_position);
	glm::mat4 orientation = glm::mat4_cast(m_world_orientation);
	glm::mat4 scale = glm::scale(glm::mat4(), m_world_scale);
	m_world_transform = position * orientation * scale;

	/* The uniforms are now out of date. */
	m_uniforms_outdated = true;

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
