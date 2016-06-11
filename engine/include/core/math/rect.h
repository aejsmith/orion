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
 * @brief               2D rectangle structure.
 */

#pragma once

#include "core/defs.h"

/** Structure defining a 2D rectangle. */
template <typename T>
struct RectImpl {
    T x;                    /**< X position. */
    T y;                    /**< Y position. */
    T width;                /**< Width. */
    T height;               /**< Height. */
public:
    /** Vector of type T. */
    using VecType = glm::detail::tvec2<T, glm::highp>;

    RectImpl() :
        x(0),
        y(0),
        width(0),
        height(0)
    {}

    RectImpl(T _x, T _y, T _width, T _height) :
        x(_x),
        y(_y),
        width(_width),
        height(_height)
    {}

    RectImpl(const VecType &pos, const VecType &size) :
        x(pos.x),
        y(pos.y),
        width(size.x),
        height(size.y)
    {}

    /** @return             Position of the rectangle. */
    VecType pos() const { return VecType(x, y); }
    /** @return             Size of the rectangle. */
    VecType size() const { return VecType(width, height); }

    /** Check whether the rectangle contains a point.
     * @param point         Point to check for.
     * @return              Whether the point is within the rectangle. */
    bool contains(const VecType &point) const {
        return (point.x >= x && point.y >= y && point.x < (x + width) && point.y < (y + height));
    }

    /** Compare for equality with another rectangle.
     * @param other         Rectangle to compare with.
     * @return              Whether they are equal. */
    bool operator ==(const RectImpl &other) const {
        return (x == other.x && y == other.y && width == other.width && height == other.height);
    }

    /** Compare for inequality with another rectangle.
     * @param other         Rectangle to compare with.
     * @return              Whether they are not equal. */
    bool operator !=(const RectImpl &other) const {
        return !(*this == other);
    }
};

/** Rectangle using single precision floating point values. */
using Rect = RectImpl<float>;

/** Rectangle using integer values. */
using IntRect = RectImpl<int32_t>;
