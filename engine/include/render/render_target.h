/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Render target base class.
 */

#ifndef ORION_RENDER_RENDER_TARGET_H
#define ORION_RENDER_RENDER_TARGET_H

#include "core/defs.h"

#include <list>

struct SceneView;

/**
 * Base render target class.
 *
 * This class is the base of a render target, either the main window or a
 * render texture. A render target is given a rendering priority to determine
 * the order in which targets will be updated, for example to ensure that
 * render textures used in the scene are updated before the main window is
 * rendered. The Engine class maintains a list of active render targets and the
 * rendering loop will update them all ordered by their priority. Each render
 * target maintains a list of SceneViews targeting them. A render target is
 * active if at least one SceneView targets it.
 */
class RenderTarget {
public:
	/** Rendering priorities. */
	enum {
		/** High priority render texture (rendered first). */
		kTextureHighPriority,
		/** Medium priority render texture. */
		kTextureMediumPriority,
		/** Low priority render texture. */
		kTextureLowPriority,
		/** Main window (rendered last). */
		kWindowPriority,
	};

	/** Type of the registered view list. */
	typedef std::list<SceneView *> SceneViewList;
public:
	virtual ~RenderTarget();

	/** @return		Size of the render target (in pixels). */
	virtual glm::ivec2 size() const = 0;

	void add_view(SceneView *view);
	void remove_view(SceneView *view);

	/** @return		Rendering priority. */
	unsigned priority() const { return m_priority; }
	/** @return		List of registered scene views. */
	const SceneViewList &views() const { return m_views; }
protected:
	explicit RenderTarget(unsigned priority);
private:
	unsigned m_priority;		/**< Rendering priority. */
	SceneViewList m_views;		/**< Registered views. */
};

#endif /* ORION_RENDER_RENDER_TARGET_H */
