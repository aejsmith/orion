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

#pragma once

#include "gpu/resource.h"

#include "render_core/uniform_buffer.h"

/** Per-view uniform buffer structure. */
UNIFORM_STRUCT_BEGIN(ViewUniforms)
    UNIFORM_STRUCT_MEMBER(glm::mat4, view);
    UNIFORM_STRUCT_MEMBER(glm::mat4, projection);
    UNIFORM_STRUCT_MEMBER(glm::mat4, viewProjection);
    UNIFORM_STRUCT_MEMBER(glm::mat4, inverseViewProjection);
    UNIFORM_STRUCT_MEMBER(glm::ivec2, viewportPosition);
    UNIFORM_STRUCT_MEMBER(glm::ivec2, viewportSize);
    UNIFORM_STRUCT_MEMBER(glm::vec3, position);
UNIFORM_STRUCT_END;

/**
 * A view into a world.
 *
 * This class represents a view into a scene: a viewing transformation, a
 * projection transformation, and a viewport rectangle. It also holds a uniform
 * buffer containing the view's parameters that can be passed to shaders.
 */
class RenderView {
public:
    RenderView();
    ~RenderView();

    void setTransform(const glm::vec3 &position, const glm::quat &orientation);
    void perspective(float fov, float zNear, float zFar);
    void setViewport(const IntRect &viewport);

    /** @return             Current position. */
    const glm::vec3 &position() const { return m_position; }
    /** @return             Current orientation. */
    const glm::quat &orientation() const { return m_orientation; }

    const glm::mat4 &view();

    /** @return             Horizontal field of view. */
    float fov() const { return m_fov; }
    /** @return             Near clipping plane. */
    float zNear() const { return m_zNear; }
    /** @return             Far clipping plane. */
    float zFar() const { return m_zFar; }

    /** @return             Viewport rectangle. */
    const IntRect &viewport() const { return m_viewport; }
    /** @return             Aspect ratio. */
    float aspect() const { return m_aspect; }

    const glm::mat4 &projection();

    const glm::mat4 &viewProjection();
    const glm::mat4 &inverseViewProjection();
    const Frustum &frustum();

    GPUResourceSet *getResources();
private:
    void updateMatrices();
private:
    glm::vec3 m_position;               /**< View position. */
    glm::quat m_orientation;            /**< View orientation. */
    glm::mat4 m_view;                   /**< World-to-view matrix. */
    bool m_viewOutdated;                /**< Whether the view matrix needs updating. */

    float m_fov;                        /**< Horizontal field of view. */
    float m_zNear;                      /**< Near clipping plane. */
    float m_zFar;                       /**< Far clipping plane. */
    glm::mat4 m_projection;             /**< View-to-projection matrix. */
    bool m_projectionOutdated;          /**< Whether the projection matrix needs updating. */

    glm::mat4 m_viewProjection;         /**< Combined view-projection matrix. */
    glm::mat4 m_inverseViewProjection;  /**< Inverse view-projection matrix. */

    Frustum m_frustum;                  /**< Viewing frustum. */

    IntRect m_viewport;                 /**< Viewport rectangle in pixels. */
    float m_aspect;                     /**< Aspect ratio. */

    /** Uniform buffer containing per-view parameters. */
    UniformBuffer<ViewUniforms> m_uniforms;

    /** Resource set containing per-view resource bindings. */
    GPUResourceSetPtr m_resources;
};

/** Get the world-to-view matrix.
 * @return              World-to-view matrix. */
inline const glm::mat4 &RenderView::view() {
    updateMatrices();
    return m_view;
}

/** Get the view-to-projection matrix.
 * @return              View-to-projection matrix. */
inline const glm::mat4 &RenderView::projection() {
    updateMatrices();
    return m_projection;
}

/** Get the combined world-to-projection matrix.
 * @return              World-to-projection matrix. */
inline const glm::mat4 &RenderView::viewProjection() {
    updateMatrices();
    return m_viewProjection;
}

/** Get the inverse world-to-projection matrix.
 * @return              Inverse world-to-projection matrix. */
inline const glm::mat4 &RenderView::inverseViewProjection() {
    updateMatrices();
    return m_inverseViewProjection;
}

/** Get the viewing frustum.
 * @return              Viewing frustum. */
inline const Frustum &RenderView::frustum() {
    updateMatrices();
    return m_frustum;
}
