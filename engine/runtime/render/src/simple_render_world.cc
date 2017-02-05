/*
 * Copyright (C) 2017 Alex Smith
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
 * @brief               Simple world implementation.
 */

#include "render/render_entity.h"
#include "render/render_light.h"
#include "render/render_view.h"
#include "render/simple_render_world.h"

/** Initialise the world. */
SimpleRenderWorld::SimpleRenderWorld() {}

/** Destroy the world. */
SimpleRenderWorld::~SimpleRenderWorld() {}

/** Cull the world against the given view.
 * @param view          View to cull against.
 * @param outResults    Results structure to fill in.
 * @param flags         Culling behaviour flags. */
void SimpleRenderWorld::cull(RenderView &view, CullResults &outResults, uint32_t flags) const {
    for (RenderEntity *entity : m_entities) {
        if (Math::intersect(view.frustum(), entity->worldBoundingBox()))
            outResults.entities.emplace_back(entity);
    }

    if (flags & kCullLights) {
        for (RenderLight *light : m_lights) {
            if (!light->cull(view))
                 outResults.lights.emplace_back(light);
        }
    }
}

/** Add an entity to the world.
 * @param entity        Entity to add. */
void SimpleRenderWorld::addEntity(RenderEntity *entity) {
    m_entities.push_back(entity);
}

/** Update an entity in the world.
 * @param entity        Entity to update. */
void SimpleRenderWorld::updateEntity(RenderEntity *entity) {
    /* Nothing to do. */
}

/** Remove an entity from the world.
 * @param entity        Entity to update. */
void SimpleRenderWorld::removeEntity(RenderEntity *entity) {
    m_entities.remove(entity);
}

/** Add a light to the world.
 * @param light         Light to add. */
void SimpleRenderWorld::addLight(RenderLight *light) {
    m_lights.push_back(light);
}

/** Update a light in the world.
 * @param light         Light to update. */
void SimpleRenderWorld::updateLight(RenderLight *light) {
    /* Nothing to do. */
}

/** Remove a light from the world.
 * @param light         Light to update. */
void SimpleRenderWorld::removeLight(RenderLight *light) {
    m_lights.remove(light);
}
