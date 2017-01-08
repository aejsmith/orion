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

class RenderTarget;
class RenderView;
class RenderWorld;

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
    RenderContext(const RenderWorld *world, RenderView *view, RenderTarget *target);
    ~RenderContext();

    /** @return             World that the context is rendering. */
    const RenderWorld *world() const { return m_world; }
    /** @return             View that is being rendered from. */
    RenderView *view() const { return m_view; }
    /** @return             Target that is being rendered to. */
    RenderTarget *target() const { return m_target; }
private:
    const RenderWorld *m_world;         /**< World that the context is rendering. */
    RenderView *m_view;                 /**< View that is being rendered from. */
    RenderTarget *m_target;             /**< Target that is being rendered to. */
};
