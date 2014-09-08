/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Render target base class.
 */

#pragma once

#include "core/core.h"

#include <list>

class CameraComponent;

/**
 * Base render target class.
 *
 * This class is the base of a render target, either the main window or a
 * render texture. A render target is given a rendering priority to determine
 * the order in which targets will be updated, for example to ensure that
 * render textures used in the scene are updated before the main window is
 * rendered. The Engine class maintains a list of active render targets and the
 * rendering loop will update them all ordered by their priority. Each render
 * target maintains a list of CameraComponents targeting them. A render target
 * is active if at least one camera targets it.
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

	/** Type of the registered camera list. */
	typedef std::list<CameraComponent *> CameraList;
public:
	virtual ~RenderTarget();

	/** @return		Size of the render target (in pixels). */
	virtual glm::ivec2 size() const = 0;

	void addCamera(CameraComponent *camera);
	void removeCamera(CameraComponent *camera);

	/** @return		Rendering priority. */
	unsigned priority() const { return m_priority; }
	/** @return		List of registered cameras. */
	const CameraList &cameras() const { return m_cameras; }
protected:
	explicit RenderTarget(unsigned priority);
private:
	unsigned m_priority;		/**< Rendering priority. */
	CameraList m_cameras;		/**< Registered cameras. */
};
