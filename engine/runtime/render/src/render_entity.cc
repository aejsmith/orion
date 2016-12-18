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

#include "gpu/gpu_manager.h"

#include "render/render_entity.h"
#include "render/render_world.h"

#include "render_core/render_resources.h"

IMPLEMENT_UNIFORM_STRUCT(EntityUniforms, "entity", ResourceSets::kEntityResources);

/**
 * Initialize the entity.
 *
 * Note that properties are not initialised. They should be initialised by the
 * creator of the entity.
 */
RenderEntity::RenderEntity() :
    m_world(nullptr),
    m_flags(0)
{
    m_resources = g_gpuManager->createResourceSet(g_renderResources->entityResourceSetLayout());
    m_resources->bindUniformBuffer(ResourceSlots::kUniforms, m_uniforms.gpu());
}

/** Destroy the entity. */
RenderEntity::~RenderEntity() {
    setWorld(nullptr);
}

/** Set the world for the entity.
 * @param world         New world (null to remove). */
void RenderEntity::setWorld(RenderWorld *world) {
    if (m_world)
        m_world->removeEntity(this);

    m_world = world;

    if (m_world)
        m_world->addEntity(this);
}

/** Set the transformation of the entity.
 * @param transform     New transformation. */
void RenderEntity::setTransform(const Transform &transform) {
    m_transform = transform;

    EntityUniforms *uniforms = m_uniforms.write();
    uniforms->transform = m_transform.matrix();
    uniforms->position = m_transform.position();

    updateWorld();
}

/** Set the bounding box of the entity.
 * @param boundingBox   New bounding box. */
void RenderEntity::setBoundingBox(const BoundingBox &boundingBox) {
    m_boundingBox = boundingBox;

    updateWorld();
}

/** Flush pending updates and get resources.
 * @return              Resource set containing per-entity resources. */
GPUResourceSet *RenderEntity::getResources() {
    m_uniforms.flush();
    return m_resources;
}

/** Update the entity in the world if required. */
void RenderEntity::updateWorld() {
    /* Either transform or bounding box has changed, recalculate world bounding
     * box. */
    m_worldBoundingBox = m_boundingBox.transform(m_transform.matrix());

    if (m_world)
        m_world->updateEntity(this);
}
