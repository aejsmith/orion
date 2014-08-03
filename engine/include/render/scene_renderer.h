/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene renderer class.
 */

#ifndef ORION_RENDER_SCENE_RENDERER_H
#define ORION_RENDER_SCENE_RENDERER_H

#include "core/defs.h"

class RenderTarget;
class Scene;
class SceneView;

/** Rendering configuration. */
struct RenderConfiguration {
	/** Rendering paths. */
	enum Path {
		/** Forward rendering. */
		kForwardPath,
		/** Deferred lighting. */
		kDeferredPath,
	};
public:
	Path path;			/**< Rendering path to use. */
};

/** Class to render a scene. */
class SceneRenderer {
public:
	static SceneRenderer *create(Scene *scene, RenderTarget *target, const RenderConfiguration &config);

	virtual ~SceneRenderer() {}

	/** Render the scene.
	 * @param view		View to render from. */
	virtual void render(SceneView *view) = 0;
protected:
	SceneRenderer(Scene *scene, RenderTarget *target, const RenderConfiguration &config);
protected:
	Scene *m_scene;			/**< Scene being rendered. */
	RenderTarget *m_target;		/**< Render target. */
	RenderConfiguration m_config;	/**< Rendering configuration. */
};

#endif /* ORION_RENDER_SCENE_RENDERER_H */
