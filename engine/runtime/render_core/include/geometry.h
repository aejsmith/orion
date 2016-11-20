/*
 * Copyright (C) 2015 Alex Smith
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
 * @brief               Geometry class.
 */

#pragma once

#include "gpu/index_data.h"
#include "gpu/vertex_data.h"

/**
 * A set of geometry (vertex/index data and primitive type).
 *
 * This structure bundles together vertex data, optional index data and a
 * primitive type for rendering. It holds only raw pointers to the vertex/index
 * data objects, as it is expected that a reference is held elsewhere.
 */
struct Geometry {
    GPUVertexData *vertices;            /**< Vertex data. */
    GPUIndexData *indices;              /**< Index data. */
    PrimitiveType primitiveType;        /**< Primitive type to render. */
};
