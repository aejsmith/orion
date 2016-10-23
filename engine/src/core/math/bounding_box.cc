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

#include "core/math/bounding_box.h"

/** Transform the bounding box.
 * @param matrix        Matrix to transform by.
 * @return              Transformed bounding box. */
BoundingBox BoundingBox::transform(const glm::mat4 &matrix) const {
    glm::vec3 xa = glm::vec3(matrix[0]) * this->minimum.x;
    glm::vec3 xb = glm::vec3(matrix[0]) * this->maximum.x;

    glm::vec3 ya = glm::vec3(matrix[1]) * this->minimum.y;
    glm::vec3 yb = glm::vec3(matrix[1]) * this->maximum.y;

    glm::vec3 za = glm::vec3(matrix[2]) * this->minimum.z;
    glm::vec3 zb = glm::vec3(matrix[2]) * this->maximum.z;

    glm::vec3 minimum(glm::min(xa, xb) + glm::min(ya, yb) + glm::min(za, zb) + glm::vec3(matrix[3]));
    glm::vec3 maximum(glm::max(xa, xb) + glm::max(ya, yb) + glm::max(za, zb) + glm::vec3(matrix[3]));
    return BoundingBox(minimum, maximum);
}
