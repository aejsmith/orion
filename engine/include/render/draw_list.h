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

#pragma once

#include "gpu/resource.h"

#include "render/geometry.h"
#include "render/scene_entity.h"

#include "shader/pass.h"

#include <list>

/**
 * Draw call structure.
 *
 * This structure is stored in a DrawList. It stores information for a single
 * rendering pass for an entity.
 */
struct DrawCall {
    Geometry geometry;                  /**< Geometry to draw. */
    Material *material;                 /**< Material to draw with. */
    SceneEntity *entity;                /**< Entity from which to take resources. */
    const Pass *pass;                   /**< Pass to draw with. */
};

/** Class storing a list of draw calls. */
class DrawList {
public:
    void addDrawCall(const Geometry &geometry, Material *material, SceneEntity *entity, const Pass *pass);

    void addDrawCalls(const Geometry &geometry, Material *material, SceneEntity *entity, Pass::Type passType);
    void addDrawCalls(SceneEntity *entity, Pass::Type passType);

    void draw(SceneLight *light = nullptr) const;

    /** @return             Whether the draw list is empty. */
    bool empty() const { return m_drawCalls.empty(); }
private:
    /** List of draw calls. */
    std::list<DrawCall> m_drawCalls;
};
