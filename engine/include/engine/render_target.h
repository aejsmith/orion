/*
 * Copyright (C) 2015 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               Render target base class.
 */

#pragma once

#include "core/core.h"

#include <list>

class RenderTarget;

struct GPUTextureImageRef;

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
    /** Standard rendering priority values. */
    enum {
        /** Default camera. */
        kCameraPriority = 0,
        /** GUI. */
        kGUIPriority = 90,
        /** Debug overlay. */
        kDebugOverlayPriority = 100,
    };

    virtual ~RenderLayer();

    void setRenderTarget(RenderTarget *target);
    void setViewport(const Rect &viewport);
    void setRenderPriority(unsigned priority);

    /** @return             Render target. */
    RenderTarget *renderTarget() const { return m_renderTarget; }
    /** @return             Normalized viewport rectangle. */
    const Rect &viewport() const { return m_viewport; }
    /** @return             Pixel (screen-space) viewport rectangle. */
    const IntRect &pixelViewport() const { return m_pixelViewport; }
    /** @return             Rendering priority. */
    unsigned renderPriority() const { return m_priority; }

    /**
     * Render the layer.
     *
     * Renders the layer. It is up to this function to set the render target
     * and viewport, as well as to clear if necessary or configure blending
     * between this layer and the previous layer.
     */
    virtual void render() = 0;
protected:
    explicit RenderLayer(unsigned priority);

    void registerRenderLayer();
    void unregisterRenderLayer();

    /** Called when the viewport is changed. */
    virtual void viewportChanged() {}
private:
    RenderTarget *m_renderTarget;   /**< Render target for the camera. */
    Rect m_viewport;                /**< Normalized viewport rectangle. */
    IntRect m_pixelViewport;        /**< Pixel viewport coordinates. */
    unsigned m_priority;            /**< Rendering priority. */
    bool m_registered;              /**< Whether the layer is registered. */
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
    using LayerList = std::list<RenderLayer *>;

    virtual ~RenderTarget();

    /** @return             Width of the render target (in pixels). */
    virtual uint32_t width() const = 0;
    /** @return             Height of the render target (in pixels). */
    virtual uint32_t height() const = 0;

    /** @return             Rendering priority. */
    unsigned priority() const { return m_priority; }

    void addLayer(RenderLayer *layer);
    void removeLayer(RenderLayer *layer);

    void render();

    /**
     * Set the render target as the current.
     *
     * Sets this render target the current render target. Note that most
     * RenderTarget objects do not have their own depth buffer (only the main
     * window does), therefore this should only be used with depth testing and
     * writes disabled. In most cases, rendering to a render target should be
     * done on temporary buffers and blitted onto the target.
     *
     * @param viewport      Optional viewport rectangle, if null viewport is set
     *                      to the render target dimensions.
     */
    virtual void set(const IntRect *viewport) = 0;

    /**
     * Get the target GPU texture image reference.
     *
     * Gets a GPU texture image reference referring to the render target. This
     * is only suitable for use as a blit target, do not attempt to use it to
     * set the render target.
     *
     * @param ref           Image reference structure to fill in.
     */
    virtual void gpu(GPUTextureImageRef &ref) = 0;
protected:
    explicit RenderTarget(unsigned priority);
private:
    unsigned m_priority;            /**< Rendering priority. */
    LayerList m_layers;             /**< Registered layers. */
};
