/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene renderer class.
 */

#pragma once

#include "core/core.h"

class RenderTarget;
class Scene;
class SceneView;

/** Rendering path enumeration. */
enum class RenderPath {
	kForward,			/**< Forward rendering. */
	kDeferred,			/**< Deferred lighting. */
};

/** Class to render a scene. */
class SceneRenderer {
public:
	static SceneRenderer *create(Scene *scene, SceneView *view, RenderTarget *target, RenderPath path);

	virtual ~SceneRenderer() {}

	/** Set the render target.
	 * @param target	New render target. */
	void setTarget(RenderTarget *target) { m_target = target; }

	/** @return		Render path this renderer implements. */
	virtual RenderPath path() const = 0;

	/** Render the scene. */
	virtual void render() = 0;
protected:
	SceneRenderer(Scene *scene, SceneView *view, RenderTarget *target);
protected:
	Scene *m_scene;			/**< Scene being rendered. */
	SceneView *m_view;		/**< View into the scene to render from. */
	RenderTarget *m_target;		/**< Render target. */
};
