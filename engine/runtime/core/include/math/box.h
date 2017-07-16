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
 * @brief               3D box structure.
 */

#pragma once

#include "core/defs.h"

/** Structure defining a 3D box. */
template <typename T>
struct BoxImpl {
    T x;                    /**< X position. */
    T y;                    /**< Y position. */
    T z;                    /**< Z position. */
    T width;                /**< Width. */
    T height;               /**< Height. */
    T depth;                /**< Depth. */
public:
    /** Vector of type T. */
    using VecType = glm::tvec3<T, glm::highp>;

    BoxImpl() :
        x      (0),
        y      (0),
        z      (0),
        width  (0),
        height (0),
        depth  (0)
    {}

    BoxImpl(T _x, T _y, T _z, T _width, T _height, T _depth) :
        x      (_x),
        y      (_y),
        z      (_z),
        width  (_width),
        height (_height),
        depth  (_depth)
    {}

    BoxImpl(const VecType &pos, const VecType &size) :
        x      (pos.x),
        y      (pos.y),
        z      (pos.z),
        width  (size.x),
        height (size.y),
        depth  (size.z)
    {}

    /** @return             Position of the box. */
    VecType pos() const { return VecType(x, y, z); }
    /** @return             Size of the box. */
    VecType size() const { return VecType(width, height, depth); }

    /** Check whether the box contains a point.
     * @param point         Point to check for.
     * @return              Whether the point is within the box. */
    bool contains(const VecType &point) const {
        return point.x >= x && point.y >= y && point.z >= z &&
               point.x < (x + width) && point.y < (y + height) && point.z < (z + depth);
    }

    /** Compare for equality with another box.
     * @param other         Box to compare with.
     * @return              Whether they are equal. */
    bool operator ==(const BoxImpl &other) const {
        return x == other.x && y == other.y && z == other.z &&
               width == other.width && height == other.height && depth == other.depth;
    }

    /** Compare for inequality with another box.
     * @param other         Box to compare with.
     * @return              Whether they are not equal. */
    bool operator !=(const BoxImpl &other) const {
        return !(*this == other);
    }
};

/** Box using single precision floating point values. */
using Box = BoxImpl<float>;

/** Box using integer values. */
using IntBox = BoxImpl<int32_t>;
