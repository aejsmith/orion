/*
 * Copyright (C) 2016 Alex Smith
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

#pragma once

#include "render/render_world.h"

/**
 * Simple renderer world implementation.
 *
 * This is a simple implementation of RenderWorld that just stores lists of
 * all the entities and lights in the world and iterates over the whole lists
 * and culls them individually.
 */
class SimpleRenderWorld : public RenderWorld {
public:
    SimpleRenderWorld();
    ~SimpleRenderWorld();

    void cull(const RenderView *view, CullResults &outResults) override;

    void addEntity(RenderEntity *entity) override;
    void updateEntity(RenderEntity *entity) override;
    void removeEntity(RenderEntity *entity) override;

    void addLight(RenderLight *light) override;
    void updateLight(RenderLight *light) override;
    void removeLight(RenderLight *light) override;
private:
    /** List of entities in the world. */
    std::list<RenderEntity *> m_entities;

    /** List of registered lights. */
    std::list<RenderLight *> m_lights;
};
