/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Renderer base component.
 */

#ifndef ORION_WORLD_RENDERER_H
#define ORION_WORLD_RENDERER_H

#include "render/scene_entity.h"

#include "world/component.h"

/**
 * Base class for a component which renders something.
 *
 * This class is the base class for components which render something in the
 * world. It implements the functionality to add SceneEntities to the renderer
 * and keeps them updated.
 */
class RendererComponent : public Component {
public:
	ORION_COMPONENT(Component::kRendererType);
public:
	~RendererComponent();
protected:
	explicit RendererComponent(Entity *entity);

	void transformed();
	void activated();
	void deactivated();

	/**
	 * Create scene entities.
	 *
	 * This function is called the first time the component is activated in
	 * the world to create the SceneEntities which will be added to the
	 * renderer. The entities' transformations will be set after this has
	 * been called.
	 *
	 * @param entities	List to populate.
	 */
	virtual void create_scene_entities(SceneEntityList &entities) = 0;
private:
	/** List of scene entities. */
	SceneEntityList m_scene_entities;
};

#endif /* ORION_WORLD_RENDERER_H */
