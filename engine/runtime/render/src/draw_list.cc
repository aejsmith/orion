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
 * @brief               Draw list class.
 */

#include "render/draw_list.h"
#include "render/render_entity.h"

#include "render_core/geometry.h"
#include "render_core/material.h"
#include "render_core/shader.h"

DrawList::DrawList() {}
DrawList::~DrawList() {}

/** Add draw calls for an entity to the list.
 * @param entity        Entity to add.
 * @param passType      Pass type to add. */
void DrawList::add(RenderEntity *entity, const std::string &passType) {
    Shader *shader = entity->material()->shader();

    for (size_t i = 0; i < shader->numPasses(passType); i++) {
        m_draws.emplace_back();
        Draw &draw = m_draws.back();

        draw.entity = entity;
        draw.pass = shader->getPass(passType, i);
    }
}

/** Perform all draw calls in the list.
 * @param cmdList       GPU command list to draw on.
 * @param variation     Shader variation to use. */
void DrawList::draw(GPUCommandList *cmdList, const ShaderKeywordSet &variation) {
    /* TODO: Track current pass/material/entity/etc, minimize the state changes. */
    for (const Draw &draw : m_draws) {
        GPU_CMD_DEBUG_GROUP(cmdList, "%s", draw.entity->name.c_str());

        cmdList->bindResourceSet(ResourceSets::kEntityResources, draw.entity->getResources());

        draw.entity->material()->setDrawState(cmdList);
        draw.pass->setDrawState(cmdList, variation);

        Geometry geometry = draw.entity->geometry();
        cmdList->draw(geometry.primitiveType, geometry.vertices, geometry.indices);
    }
}
