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
 * @brief               Scene entity base class.
 */

#pragma once

#include "gpu/resource.h"

#include "shader/uniform_buffer.h"

struct Geometry;

class Material;
class Scene;

/** Per-entity uniform buffer structure. */
UNIFORM_STRUCT_BEGIN(EntityUniforms)
    UNIFORM_STRUCT_MEMBER(glm::mat4, transform);
    UNIFORM_STRUCT_MEMBER(glm::vec3, position);
UNIFORM_STRUCT_END;

/**
 * Base class for a scene entity.
 *
 * This class is the base for a renderable entity in the scene. Each Entity in
 * the world which has a rendering component attached will add one or more
 * SceneEntities to the renderer's scene in order for them to be rendered.
 */
class SceneEntity {
public:
    virtual ~SceneEntity();

    /** @return             Current transformation. */
    const Transform &transform() const { return m_transform; }
    /** @return             Current position. */
    const glm::vec3 &position() const { return m_transform.position(); }
    /** @return             Current orientation. */
    const glm::quat &orientation() const { return m_transform.orientation(); }
    /** @return             Current scale. */
    const glm::vec3 &scale() const { return m_transform.scale(); }
    /** @return             Local-space bounding box. */
    const BoundingBox &boundingBox() const { return m_boundingBox; }
    /** @return             World-space bounding box. */
    const BoundingBox &worldBoundingBox() const { return m_worldBoundingBox; }
    /** @return             Whether the rendered object casts a shadow. */
    bool castShadow() const { return m_castShadow; }

    void setTransform(const Transform &transform);
    void setBoundingBox(const BoundingBox &boundingBox);
    void setCastShadow(bool castShadow);

    /** Flush pending updates and get resources for a draw call.
     * @return              Resource set containing per-entity resources. */
    GPUResourceSet *resourcesForDraw() {
        m_uniforms.flush();
        return m_resources;
    }

    /** Get the geometry for the entity.
     * @param geometry      Geometry structure to fill in. */
    virtual void geometry(Geometry &geometry) const = 0;

    /** Get the material for the entity.
     * @return              Material for the entity. */
    virtual Material *material() const = 0;
protected:
    SceneEntity();
private:
    void queueUpdate();
private:
    Transform m_transform;          /**< Transformation of the entity. */
    BoundingBox m_boundingBox;      /**< Local-space bounding box. */
    BoundingBox m_worldBoundingBox; /**< World-space bounding box. */
    bool m_castShadow;              /**< Whether the rendered object casts a shadow. */

    Scene *m_scene;                 /**< Scene the entity belongs to. */
    bool m_updatePending;           /**< Whether an update in the Scene is pending. */

    /** Uniform buffer containing per-entity parameters. */
    UniformBuffer<EntityUniforms> m_uniforms;

    /** Resource set containing per-entity resources. */
    GPUResourceSetPtr m_resources;

    friend class Scene;
};
