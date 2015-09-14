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
 * @brief               Axis-aligned bounding box class.
 */

#pragma once

#include "core/defs.h"

/** Axis-aligned bounding box. */
struct BoundingBox {
    glm::vec3 minimum;              /**< Minimum coordinate. */
    glm::vec3 maximum;              /**< Maximum coordinate. */
public:
    /** Initialise an empty bounding box. */
    BoundingBox() :
        minimum(0.0f),
        maximum(0.0f)
    {}

    /** Initialise from a minimum/maximum position.
     * @param _minimum      Minimum position.
     * @param _maximum      Maximum position. */
    BoundingBox(const glm::vec3 &_minimum, const glm::vec3 &_maximum) :
        minimum(_minimum),
        maximum(_maximum)
    {}

    /** Compare this bounding box with another. */
    bool operator ==(const BoundingBox &other) const {
        return this->minimum == other.minimum && this->maximum == other.maximum;
    }

    BoundingBox transform(const glm::mat4 &matrix) const;

    void debugDraw(const glm::vec4 &colour, bool perView = false) const;
};