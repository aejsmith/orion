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
 * @brief               Renderer world class.
 */

#pragma once

#include "core/core.h"

#include <list>

class RenderEntity;
class RenderLight;
class RenderView;

/**
 * Renderer's view of world.
 *
 * This class maintains the renderer's view of the world for the purposes of
 * culling, etc. This is only a base interface, implementation is left to
 * derived classes so that there can be different implementations optimised for
 * different use cases.
 */
class RenderWorld {
public:
    /** Culling behaviour flags. */
    enum CullFlags : uint32_t {
        /** Whether to visible lights in the results. */
        kCullLights = (1 << 0),
    };

    /** Structure containing the results of culling. */
    struct CullResults {
        /** List of visible entities. */
        std::list<RenderEntity *> entities;

        /** List of visible lights. */
        std::list<RenderLight *> lights;
    };

    virtual ~RenderWorld() {}

    /**
     * Cull the world against the given view.
     *
     * Given a view, obtains lists of all the entities visible from it, as well
     * as all the lights visible if the kCullLights flag is passed.
     *
     * @param view          View to cull against.
     * @param outResults    Results structure to fill in.
     * @param flags         Culling behaviour flags.
     */
    virtual void cull(
        RenderView &view,
        CullResults &outResults,
        uint32_t flags = kCullLights) const = 0;

    /** Add an entity to the world.
     * @param entity        Entity to add. */
    virtual void addEntity(RenderEntity *entity) = 0;

    /** Update an entity in the world.
     * @param entity        Entity to update. */
    virtual void updateEntity(RenderEntity *entity) = 0;

    /** Remove an entity from the world.
     * @param entity        Entity to update. */
    virtual void removeEntity(RenderEntity *entity) = 0;

    /** Add a light to the world.
     * @param light         Light to add. */
    virtual void addLight(RenderLight *light) = 0;

    /** Update a light in the world.
     * @param light         Light to update. */
    virtual void updateLight(RenderLight *light) = 0;

    /** Remove a light from the world.
     * @param light         Light to update. */
    virtual void removeLight(RenderLight *light) = 0;
protected:
    RenderWorld() {}
};
