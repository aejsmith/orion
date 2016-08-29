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
 * @brief               Draw list class.
 */

#include "render/draw_list.h"

#include "shader/material.h"

/** Add a draw call to the list.
 * @param geometry      Geometry to draw.
 * @param material      Material to draw with.
 * @param entity        Entity from which to take resources.
 * @param pass          Pass to draw with. */
void DrawList::addDrawCall(const Geometry &geometry, Material *material, SceneEntity *entity, const Pass *pass) {
    m_drawCalls.emplace_back();

    DrawCall &drawCall = m_drawCalls.back();
    drawCall.geometry = geometry;
    drawCall.material = material;
    drawCall.entity = entity;
    drawCall.pass = pass;
}

/** Add draw calls for all passes to the list.
 * @param geometry      Geometry to draw.
 * @param material      Material to draw with.
 * @param entity        Entity from which to take resources.
 * @param passType      Pass type to use. */
void DrawList::addDrawCalls(const Geometry &geometry, Material *material, SceneEntity *entity, Pass::Type passType) {
    Shader *shader = material->shader();
    for (size_t i = 0; i < shader->numPasses(passType); i++) {
        const Pass *pass = shader->pass(passType, i);
        addDrawCall(geometry, material, entity, pass);
    }
}

/** Add draw calls for all passes to the list.
 * @param entity        Entity to draw.
 * @param passType      Pass type to add. */
void DrawList::addDrawCalls(SceneEntity *entity, Pass::Type passType) {
    Geometry geometry;
    entity->geometry(geometry);

    addDrawCalls(geometry, entity->material(), entity, passType);
}

/** Perform all draw calls in the list.
 * @param light         Light being rendered with (can be null). */
void DrawList::draw(SceneLight *light) const {
    /* TODO: Track current pass/material/etc, minimize the state changes. */
    for (const DrawCall &drawCall : m_drawCalls) {
        if (drawCall.entity) {
            GPU_BEGIN_DEBUG_GROUP("%s", drawCall.entity->name.c_str());
            g_gpuManager->bindResourceSet(ResourceSets::kEntityResources, drawCall.entity->resourcesForDraw());
        }

        drawCall.material->setDrawState();
        drawCall.pass->setDrawState(light);

        g_gpuManager->draw(
            drawCall.geometry.primitiveType,
            drawCall.geometry.vertices,
            drawCall.geometry.indices);

        if (drawCall.entity)
            GPU_END_DEBUG_GROUP();
    }
}
