/*
 * Copyright (C) 2015-2017 Alex Smith
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
 */

#pragma once

#include "engine/component.h"
#include "engine/render_target.h"

#include "render/render_pipeline.h"
#include "render/render_view.h"

/** A view into the world from which the scene will be rendered. */
class Camera : public Component, public RenderLayer {
public:
    CLASS();

    Camera();

    /**
     * Renderer settings.
     */

    /** Rendering pipeline for the camera. */
    PROPERTY() ObjectPtr<RenderPipeline> renderPipeline;

    /**
     * View settings.
     */

    /** @return             World-to-view matrix. */
    const glm::mat4 &view() { return m_renderView.view(); }

    /**
     * Projection settings.
     */

    /** Type of the projection. */
    enum class ProjectionMode {
        kPerspective,
        //kOrthographic,
    };

    VPROPERTY(ProjectionMode, projectionMode);
    VPROPERTY(float, fov, "get": "fov", "set": "setFOV");
    VPROPERTY(float, zNear);
    VPROPERTY(float, zFar);

    void setProjectionMode(ProjectionMode mode) {}
    void perspective(float fov = 75.0f, float zNear = 0.1f, float zfar = 1000.0f);
    void setFOV(float fov);
    void setZNear(float zNear);
    void setZFar(float zFar);

    /** @return             Current projection mode. */
    ProjectionMode projectionMode() const { return ProjectionMode::kPerspective; }
    /** @return             Horizontal field of view. */
    float fov() const { return m_renderView.fov(); }
    /** @return             Near clipping plane. */
    float zNear() const { return m_renderView.zNear(); }
    /** @return             Far clipping plane. */
    float zFar() const { return m_renderView.zFar(); }

    /** @return             View-to-projection matrix. */
    const glm::mat4 &projection() { return m_renderView.projection(); }
protected:
    ~Camera() {}

    void render(bool first) override;

    void transformed(unsigned changed) override;
    void activated() override;
    void deactivated() override;

    #ifdef ORION_BUILD_DEBUG
    std::string renderLayerName() const override;
    #endif
private:
    void viewportChanged() override;

    RenderView m_renderView;            /**< View implementing this camera. */
};

/** Set up a perspective projection.
 * @param fovx          Horizontal field of view, in degrees.
 * @param znear         Distance to near clipping plane.
 * @param zfar          Distance to far clipping plane. */
inline void Camera::perspective(float fovx, float znear, float zfar) {
    m_renderView.perspective(fovx, znear, zfar);
}

/** Set the horizontal field of view.
 * @param fov           New horizontal FOV, in degrees. */
inline void Camera::setFOV(float fov) {
    m_renderView.perspective(fov, zNear(), zFar());
}

/** Set the near clipping plane.
 * @param zNear         New distance to the near clipping plane. */
inline void Camera::setZNear(float zNear) {
    m_renderView.perspective(fov(), zNear, zFar());
}

/** Set the far clipping plane.
 * @param zfar          New distance to the far clipping plane. */
inline void Camera::setZFar(float zFar) {
    m_renderView.perspective(fov(), zNear(), zFar);
}
