/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Renderer base component.
 */

#pragma once

#include "render/scene_entity.h"

#include "world/component.h"

#include <list>

/**
 * Base class for a component which renders something.
 *
 * This class is the base class for components which render something in the
 * world. It implements the functionality to add SceneEntities to the renderer
 * and keeps them updated.
 */
class RendererComponent : public Component {
public:
	DECLARE_COMPONENT(Component::kRendererType);
public:
	~RendererComponent();
protected:
	/** Type of a scene entity list. */
	typedef std::list<SceneEntity *> SceneEntityList;
protected:
	explicit RendererComponent(Entity *entity);

	void transformed() override;
	void activated() override;
	void deactivated() override;

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
	virtual void createSceneEntities(SceneEntityList &entities) = 0;
private:
	/** List of scene entities. */
	SceneEntityList m_sceneEntities;
};
