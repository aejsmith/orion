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
 * @brief               Rendering pipeline class.
 */

#pragma once

#include "engine/object.h"

#include "render/render_context.h"

/**
 * Rendering pipeline base class.
 *
 * This class is the base for a rendering pipeline, which implements the process
 * for rendering the world.
 */
class RenderPipeline : public Object {
public:
    CLASS();

    ~RenderPipeline() {}

    /**
     * Render a world.
     *
     * Renders the given world from a view to a render target. This is expected
     * to set up a RenderContext (or derived class) based on these parameters,
     * and then use methods on that to render the world.
     *
     * @param world         World to render.
     * @param view          View to render from.
     * @param target        Render target.
     */
    virtual void render(const RenderWorld &world, RenderView &view, RenderTarget &target) = 0;
protected:
    RenderPipeline() {}
};