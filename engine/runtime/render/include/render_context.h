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
 * @brief               Rendering context class.
 */

#pragma once

#include "core/core.h"

#include "render/render_world.h"

class RenderTarget;
class RenderView;

/**
 * Rendering context class.
 *
 * This class manages per-frame rendering state for a RenderPipeline. The
 * pipeline should create an instance of this class (or a derived class which
 * includes extra pipeline-specific state) and then use methods on it to perform
 * its rendering.
 */
class RenderContext {
public:
    RenderContext(const RenderWorld &world, RenderView &view, RenderTarget &target);
    ~RenderContext();

    /** @return             World that the context is rendering. */
    const RenderWorld &world() const { return m_world; }
    /** @return             View that is being rendered from. */
    RenderView &view() const { return m_view; }
    /** @return             Target that is being rendered to. */
    RenderTarget &target() const { return m_target; }

    /**
     * Cull the world against the primary view.
     *
     * Obtains lists of all the entities visible from the primary view, as well
     * as all the lights visible if the kCullLights flag is passed.
     *
     * @param outResults    Results structure to fill in.
     * @param flags         Culling behaviour flags.
     */
    void cull(
        RenderWorld::CullResults &outResults,
        uint32_t flags = RenderWorld::kCullLights) const
    {
        m_world.cull(m_view, outResults, flags);
    }

    /**
     * Cull the world against the given view.
     *
     * Obtains lists of all the entities visible from the given view, as well
     * as all the lights visible if the kCullLights flag is passed.
     *
     * @param view          View to cull against.
     * @param outResults    Results structure to fill in.
     * @param flags         Culling behaviour flags.
     */
    void cull(
        RenderView &view,
        RenderWorld::CullResults &outResults,
        uint32_t flags = RenderWorld::kCullLights) const
    {
        m_world.cull(view, outResults, flags);
    }
private:
    const RenderWorld &m_world;         /**< World that the context is rendering. */
    RenderView &m_view;                 /**< View that is being rendered from. */
    RenderTarget &m_target;             /**< Target that is being rendered to. */
};
