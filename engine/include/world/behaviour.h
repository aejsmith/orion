/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Behaviour component.
 */

#pragma once

#include "world/component.h"

/**
 * Behaviour component class.
 *
 * This class is the base class for adding any custom behaviour to an entity.
 * It should be derived from rather than deriving directly from Component as it
 * includes the necessary boilerplate to set the component type. Custom
 * behaviour can be implemented via the Component hook functions.
 */
class BehaviourComponent : public Component {
public:
	DECLARE_COMPONENT(Component::kBehaviourType);
protected:
	BehaviourComponent(Entity *entity) : Component(Component::kBehaviourType, entity) {}
	~BehaviourComponent() {}
};
