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
 * @brief               OpenGL resource set implementation.
 */

#pragma once

#include "gl.h"

/** Maximum number of resource sets supported. */
static const size_t kGLMaxResourceSets = 6;

/** OpenGL resource set layout implementation. */
class GLResourceSetLayout : public GPUResourceSetLayout {
public:
    GLResourceSetLayout(GPUResourceSetLayoutDesc &&desc);

    unsigned mapSlot(unsigned set, unsigned slot) const;
private:
    /**
     * Mapping of slot index GL binding points.
     *
     * This is used to map from a slot index in this resource set to a type-
     * specific binding point index. The value given by this is an offset from
     * the start of the set's binding point range. For further details, see
     * resource.cc.
     */
    std::vector<size_t> m_mapping;
};
