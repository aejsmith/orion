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
 * @brief               Scene entity base class.
 */

#include "render/scene_entity.h"

#include "shader/slots.h"

IMPLEMENT_UNIFORM_STRUCT(EntityUniforms, "entity", UniformSlots::kEntityUniforms);

/**
 * Initialize the entity.
 *
 * Note that properties are not initialised. They should be initialised by the
 * creator of the entity.
 */
SceneEntity::SceneEntity() {}

/** Destroy the entity. */
SceneEntity::~SceneEntity() {}

/** Set whether the rendered object casts a shadow.
 * @param castShadow    Whether the rendered object casts a shadow. */
void SceneEntity::setCastShadow(bool castShadow) {
    m_castShadow = castShadow;
}

/** Private function called from Scene to set the transformation.
 * @param transform     New transformation. */
void SceneEntity::setTransform(const Transform &transform) {
    m_transform = transform;

    m_uniforms->transform = m_transform.matrix();
    m_uniforms->position = m_transform.position();
}
