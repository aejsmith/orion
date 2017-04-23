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
 * @brief               Renderer entity class.
 */

#pragma once

#include "gpu/resource.h"

#include "render_core/uniform_buffer.h"

struct Geometry;

class Material;
class RenderWorld;

/** Per-entity uniform buffer structure. */
UNIFORM_STRUCT_BEGIN(EntityUniforms)
    UNIFORM_STRUCT_MEMBER(glm::mat4, transform);
    UNIFORM_STRUCT_MEMBER(glm::vec3, position);
UNIFORM_STRUCT_END;

/**
 * Base class for a renderable entity.
 *
 * This class is the base for a renderable entity in the world. Each Entity in
 * the world which has a rendering component attached will add one or more
 * RenderEntity instances to the world's render system in order for them to be
 * rendered.
 */
class RenderEntity {
public:
    /** Entity flags. */
    enum : uint32_t {
        /** Whether the entity casts a shadow. */
        kCastsShadow = (1 << 0),
    };

    virtual ~RenderEntity();

    void setWorld(RenderWorld *world);

    void setTransform(const Transform &transform);
    void setBoundingBox(const BoundingBox &boundingBox);

    /** Set the flags for the entity.
     * @param flags         New flags. */
    void setFlags(uint32_t flags) { m_flags = flags; }

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

    /** @return             Flags for the entity. */
    uint32_t flags() const { return m_flags; }
    /** @return             Whether the entity casts a shadow. */
    bool castsShadow() const { return (m_flags & kCastsShadow) != 0; }

    GPUResourceSet *getResources();

    /** Get the geometry for the entity.
     * @return              Geometry structure to fill in. */
    virtual Geometry geometry() const = 0;

    /** Get the material for the entity.
     * @return              Material for the entity. */
    virtual Material *material() const = 0;

    std::string name;                   /**< Name of the entity. */
protected:
    RenderEntity();

    void updateWorld();
private:
    RenderWorld *m_world;               /**< World that this entity belongs to. */

    Transform m_transform;              /**< Transformation of the entity. */
    BoundingBox m_boundingBox;          /**< Local-space bounding box. */
    BoundingBox m_worldBoundingBox;     /**< World-space bounding box. */
    uint32_t m_flags;                   /**< Behaviour flags for the entity. */

    /** Uniform buffer containing per-entity parameters. */
    UniformBuffer<EntityUniforms> m_uniforms;

    /** Resource set containing per-entity resources. */
    GPUResourceSetPtr m_resources;
};
