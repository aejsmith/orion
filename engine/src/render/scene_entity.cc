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

#include "gpu/gpu_manager.h"

#include "render/render_manager.h"
#include "render/scene.h"
#include "render/scene_entity.h"

#include "shader/resource.h"

IMPLEMENT_UNIFORM_STRUCT(EntityUniforms, "entity", ResourceSets::kEntityResources);

/**
 * Initialize the entity.
 *
 * Note that properties are not initialised. They should be initialised by the
 * creator of the entity.
 */
SceneEntity::SceneEntity() :
    m_updatePending(false)
{
    m_resources = g_gpuManager->createResourceSet(
        g_renderManager->resources().entityResourceSetLayout);
    m_resources->bindUniformBuffer(ResourceSlots::kUniforms, m_uniforms.gpu());
}

/** Destroy the entity. */
SceneEntity::~SceneEntity() {}

/** Set the transformation of the entity.
 * @param transform     New transformation. */
void SceneEntity::setTransform(const Transform &transform) {
    m_transform = transform;

    EntityUniforms *uniforms = m_uniforms.write();
    uniforms->transform = m_transform.matrix();
    uniforms->position = m_transform.position();

    queueUpdate();
}

/** Set the bounding box of the entity.
 * @param boundingBox   New bounding box. */
void SceneEntity::setBoundingBox(const BoundingBox &boundingBox) {
    m_boundingBox = boundingBox;

    queueUpdate();
}

/** Set whether the rendered object casts a shadow.
 * @param castShadow    Whether the rendered object casts a shadow. */
void SceneEntity::setCastShadow(bool castShadow) {
    m_castShadow = castShadow;
}

/** Mark the entity as requiring an update in the Scene. */
void SceneEntity::queueUpdate() {
    /* Either transform or bounding box has changed, recalculate world bounding
     * box. TODO: Could defer this until Scene performs update? */
    m_worldBoundingBox = m_boundingBox.transform(m_transform.matrix());

    if (m_scene && !m_updatePending) {
        m_updatePending = true;
        m_scene->queueEntityUpdate(this);
    }
}
