/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Render target base class.
 */

#pragma once

#include "core/core.h"

#include <list>

class RenderTarget;

/**
 * Render target layer class.
 *
 * This class is the base for something which renders to a render target, such
 * as a camera or the GUI. Layers have a viewport which defines the area of the
 * target that they cover, and ordering which defines the order in which they
 * are rendered to the target.
 */
class RenderLayer {
public:
	/** Standard layer order values. */
	enum {
		/** Default camera order. */
		kCameraLayerOrder = 0,
		/** GUI order. */
		kGUILayerOrder = 90,
		/** Console order. */
		kConsoleLayerOrder = 100,
	};
public:
	virtual ~RenderLayer();

	void setRenderTarget(RenderTarget *target);
	void setViewport(const Rect &viewport);
	void setLayerOrder(unsigned order);

	/** @return		Render target. */
	RenderTarget *renderTarget() const { return m_renderTarget; }
	/** @return		Normalized viewport rectangle. */
	const Rect &viewport() const { return m_viewport; }
	/** @return		Pixel (screen-space) viewport rectangle. */
	const IntRect &pixelViewport() const { return m_pixelViewport; }
	/** @return		Layer order. */
	unsigned layerOrder() const { return m_layerOrder; }

	/**
	 * Render the layer.
	 *
	 * Renders the layer. It is up to this function to actually set the
	 * render target and clear it if it desires, as well as configure
	 * blending between this layer and the previous layer.
	 */
	virtual void render() = 0;
protected:
	RenderLayer();

	void registerLayer();
	void unregisterLayer();

	/** Called when the render target is changed. */
	virtual void renderTargetChanged() {}

	/** Called when the viewport is changed. */
	virtual void viewportChanged() {}
private:
	RenderTarget *m_renderTarget;	/**< Render target for the camera. */
	Rect m_viewport;		/**< Normalized viewport rectangle. */
	IntRect m_pixelViewport;	/**< Pixel viewport coordinates. */
	unsigned m_layerOrder;		/**< Layer order value. */
	bool m_registered;		/**< Whether the layer is registered. */
};

/**
 * Base render target class.
 *
 * This class is the base of a render target, either the main window or a
 * render texture. A render target is given a rendering priority to determine
 * the order in which targets will be updated, for example to ensure that
 * render textures used in the scene are updated before the main window is
 * rendered. The Engine class maintains a list of active render targets and the
 * rendering loop will update them all ordered by their priority. Each render
 * target maintains a list of layers to be drawn on that target, and will be
 * updated by the rendering loop if at least one layer exists on the target.
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

	/** Type of the registered layer list. */
	typedef std::list<RenderLayer *> LayerList;
public:
	virtual ~RenderTarget();

	/** @return		Width of the render target (in pixels). */
	virtual uint32_t width() const = 0;
	/** @return		Height of the render target (in pixels). */
	virtual uint32_t height() const = 0;

	void addLayer(RenderLayer *layer);
	void removeLayer(RenderLayer *layer);

	/** @return		Rendering priority. */
	unsigned priority() const { return m_priority; }
	/** @return		List of registered layers. */
	const LayerList &layers() const { return m_layers; }
protected:
	explicit RenderTarget(unsigned priority);
private:
	unsigned m_priority;		/**< Rendering priority. */
	LayerList m_layers;		/**< Registered layers. */
};
