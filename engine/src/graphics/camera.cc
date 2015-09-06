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
 * @brief               Camera component.
 *
 * TODO:
 *  - Recalculate projection and viewport when render target is resized (add
 *    listeners to RenderTarget).
 */

#include "engine/entity.h"
#include "engine/render_target.h"
#include "engine/window.h"
#include "engine/world.h"

#include "graphics/camera.h"

#include "render/scene_renderer.h"

/**
 * Construct a default camera.
 *
 * Constructs the camera with a perspective projection with a 75 degree
 * horizontal FOV, near clipping plane of 1.0 and far clipping plane of 1000.0.
 * The default render target will be the main window.
 *
 * @param entity        Entity to attach the camera to.
 */
Camera::Camera(Entity *entity) :
    Component(Component::kCameraType, entity),
    m_sceneView(&m_postEffectChain),
    m_renderPath(RenderPath::kDeferred)
{
    /* Initialize the scene view with a default projection. */
    perspective();

    /* Default to the main window as the render target. */
    setRenderTarget(g_mainWindow);
    setLayerOrder(RenderLayer::kCameraLayerOrder);
}

/** Render the scene from the camera to its render target. */
void Camera::render() {
    SceneRenderer renderer(world()->scene(), &m_sceneView, renderTarget(), m_renderPath);
    renderer.render();
}

/** Update the viewport in the SceneView. */
void Camera::viewportChanged() {
    m_sceneView.setViewport(pixelViewport());
}

/** Called when the camera transformation is changed.
 * @param changed       Flags indicating changes made. */
void Camera::transformed(unsigned changed) {
    m_sceneView.setTransform(entity()->worldPosition(), entity()->worldOrientation());
}

/** Called when the camera becomes active in the world. */
void Camera::activated() {
    registerLayer();
}

/** Called when the camera becomes inactive in the world. */
void Camera::deactivated() {
    unregisterLayer();
}
