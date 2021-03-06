/*
 * Copyright (C) 2015-2016 Alex Smith
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
 * @brief               Renderer view class.
 */

#include "gpu/gpu_manager.h"

#include "render/render_view.h"

#include "render_core/render_resources.h"

IMPLEMENT_UNIFORM_STRUCT(ViewUniforms, "view", ResourceSets::kViewResources);

/**
 * Initialize the view.
 *
 * Initializes the view. The view is initially invalid: a transformation,
 * projection and viewport must be set manually.
 */
RenderView::RenderView() :
    m_viewOutdated       (true),
    m_projectionOutdated (true),
    m_aspect             (1.0f)
{
    m_resources = g_gpuManager->createResourceSet(g_renderResources->viewResourceSetLayout());
    m_resources->bindUniformBuffer(ResourceSlots::kUniforms, m_uniforms.gpu());
}

/** Destroy the view. */
RenderView::~RenderView() {}

/** Set the viewing transformation.
 * @param position      Viewing position.
 * @param orientation   Viewing orientation. */
void RenderView::setTransform(const glm::vec3 &position, const glm::quat &orientation) {
    m_position = position;
    m_orientation = orientation;

    m_uniforms.write()->position = m_position;

    m_viewOutdated = true;
}

/** Set a perspective projection.
 * @param fov           Horizontal field of view, in degrees.
 * @param zNear         Distance to near clipping plane.
 * @param zFar          Distance to far clipping plane. */
void RenderView::perspective(float fov, float zNear, float zFar) {
    m_fov = fov;
    m_zNear = zNear;
    m_zFar = zFar;

    m_projectionOutdated = true;
}

/** Set the viewport.
 * @param viewport      Viewport rectangle in pixels. */
void RenderView::setViewport(const IntRect &viewport) {
    if (viewport != m_viewport) {
        m_viewport = viewport;

        ViewUniforms *uniforms = m_uniforms.write();
        uniforms->viewportPosition = viewport.pos();
        uniforms->viewportSize = viewport.size();

        /* Calculate aspect ratio. If it changes, we must recalculate the projection
         * matrix. */
        float aspect = static_cast<float>(viewport.width) / static_cast<float>(viewport.height);
        if (aspect != m_aspect) {
            m_aspect = aspect;
            m_projectionOutdated = true;
        }
    }
}

/** Flush pending updates and get resources.
 * @return              Resource set containing per-view resources. */
GPUResourceSet *RenderView::getResources() {
    /* Ensure view and projection are up to date. */
    updateMatrices();
    m_uniforms.flush();
    return m_resources;
}

/** Update the view/projection matrices. */
void RenderView::updateMatrices() {
    bool wasOutdated = m_viewOutdated || m_projectionOutdated;
    ViewUniforms *uniforms = (wasOutdated) ? m_uniforms.write() : nullptr;

    if (m_viewOutdated) {
        /* Viewing matrix is a world-to-view transformation, so we want
         * the inverse of the given position and orientation. */
        glm::mat4 position = glm::translate(glm::mat4(), -m_position);
        glm::mat4 orientation = glm::mat4_cast(glm::inverse(m_orientation));

        m_view = orientation * position;
        uniforms->view = m_view;
        m_viewOutdated = false;
    }

    if (m_projectionOutdated) {
        /* Convert horizontal field of view to vertical. */
        float fov = glm::radians(m_fov);
        float verticalFOV = 2.0f * atanf(tanf(fov * 0.5f) / m_aspect);

        m_projection = glm::perspective(verticalFOV, m_aspect, m_zNear, m_zFar);
        uniforms->projection = m_projection;
        m_projectionOutdated = false;
    }

    if (wasOutdated) {
        m_viewProjection = m_projection * m_view;
        m_inverseViewProjection = glm::inverse(m_viewProjection);
        m_frustum.update(m_viewProjection, m_inverseViewProjection);

        uniforms->viewProjection = m_viewProjection;
        uniforms->inverseViewProjection = m_inverseViewProjection;
    }
}
