/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Render target class.
 */

#include "engine/engine.h"
#include "engine/render_target.h"

/**
 * Initialize the layer.
 *
 * Initializes the layer. The layer is initially not associated with a render
 * target, derived classes must set a default target and layer order themselves,
 * and register themselves with the target.
 */
RenderLayer::RenderLayer() :
	m_renderTarget(nullptr),
	m_viewport(0.0f, 0.0f, 1.0f, 1.0f),
	m_pixelViewport(0, 0, 0, 0),
	m_layerOrder(0),
	m_registered(false)
{}

/** Destroy the layer. */
RenderLayer::~RenderLayer() {
	checkMsg(!m_registered, "Destroying RenderLayer while still registered");
}

/** Set the render target.
 * @param target	New render target. */
void RenderLayer::setRenderTarget(RenderTarget *target) {
	if(m_registered)
		m_renderTarget->removeLayer(this);

	m_renderTarget = target;

	/* Recalculate pixel viewport. */
	setViewport(m_viewport);

	if(m_registered)
		m_renderTarget->addLayer(this);
}

/**
 * Set the viewport.
 *
 * Sets the viewport rectangle. Coordinates are normalized, range from (0, 0)
 * in the top left corner to (1, 1) in the bottom right corner. The actual
 * viewport rectangle is calculated automatically based on the render target's
 * dimensions.
 *
 * @param viewport	Normalized viewport rectangle.
 */
void RenderLayer::setViewport(const Rect &viewport) {
	m_viewport = viewport;

	uint32_t targetWidth = m_renderTarget->width();
	uint32_t targetHeight = m_renderTarget->height();

	m_pixelViewport = IntRect(
		m_viewport.x * static_cast<float>(targetWidth),
		m_viewport.y * static_cast<float>(targetHeight),
		m_viewport.width * static_cast<float>(targetWidth),
		m_viewport.height * static_cast<float>(targetHeight));

	viewportChanged();
}

/**
 * Set the layer order.
 *
 * Set the layer rendering order. Each layer on a render target has a order
 * value which defines the order in which layers on the target are rendered.
 * Layers with higher order values will appear on top of those with lower
 * values.
 *
 * @param order		New layer order.
 */
void RenderLayer::setLayerOrder(unsigned order) {
	/* If we are registered we have to re-register in the correct place in
	 * the target's layer list. */
	if(m_registered)
		m_renderTarget->removeLayer(this);

	m_layerOrder = order;

	if(m_registered)
		m_renderTarget->addLayer(this);
}

/** Register the layer with its render target. */
void RenderLayer::registerLayer() {
	check(m_renderTarget);
	check(!m_registered);

	m_renderTarget->addLayer(this);
	m_registered = true;
}

/** Unregister the layer from its render target. */
void RenderLayer::unregisterLayer() {
	check(m_registered);

	m_renderTarget->removeLayer(this);
	m_registered = false;
}

/** Initialize the render target.
 * @param priority	Rendering priority. */
RenderTarget::RenderTarget(unsigned priority) :
	m_priority(priority)
{}

/** Destroy the render target. */
RenderTarget::~RenderTarget() {
	checkMsg(m_layers.empty(), "Destroying RenderTarget with active layers");
}

/** Add a layer to the render target.
 * @param layer		Layer to add. */
void RenderTarget::addLayer(RenderLayer *layer) {
	bool wasEmpty = m_layers.empty();

	/* List is sorted by priority. */
	for(auto it = m_layers.begin(); it != m_layers.end(); ++it) {
		RenderLayer *exist = *it;
		if(layer->layerOrder() < exist->layerOrder()) {
			m_layers.insert(it, layer);
			return;
		}
	}

	/* Insertion point not found, add at end. */
	m_layers.push_back(layer);

	if(wasEmpty)
		g_engine->addRenderTarget(this);
}

/** Remove a layer from the render target.
 * @param layer		Layer to remove. */
void RenderTarget::removeLayer(RenderLayer *layer) {
	m_layers.remove(layer);

	if(m_layers.empty())
		g_engine->removeRenderTarget(this);
}
