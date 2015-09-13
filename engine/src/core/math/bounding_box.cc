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

#include "engine/debug_manager.h"

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

/** Draw the bounding box through the debug renderer.
 * @param colour        Colour to draw in.
 * @param perView       Whether to draw for the whole frame or just the next
 *                      view rendered. */
void BoundingBox::debugDraw(const glm::vec4 &colour, bool perView) const {
    /* Calculate corners (left/right, bottom/top, back/front). */
    glm::vec3 lbb(this->minimum.x, this->minimum.y, this->minimum.z);
    glm::vec3 lbf(this->minimum.x, this->minimum.y, this->maximum.z);
    glm::vec3 ltb(this->minimum.x, this->maximum.y, this->minimum.z);
    glm::vec3 ltf(this->minimum.x, this->maximum.y, this->maximum.z);
    glm::vec3 rbb(this->maximum.x, this->minimum.y, this->minimum.z);
    glm::vec3 rbf(this->maximum.x, this->minimum.y, this->maximum.z);
    glm::vec3 rtb(this->maximum.x, this->maximum.y, this->minimum.z);
    glm::vec3 rtf(this->maximum.x, this->maximum.y, this->maximum.z);

    /* Convert to lines. */
    g_debugManager->drawLine(lbb, rbb, colour, perView);
    g_debugManager->drawLine(rbb, rbf, colour, perView);
    g_debugManager->drawLine(rbf, lbf, colour, perView);
    g_debugManager->drawLine(lbf, lbb, colour, perView);
    g_debugManager->drawLine(ltb, rtb, colour, perView);
    g_debugManager->drawLine(rtb, rtf, colour, perView);
    g_debugManager->drawLine(rtf, ltf, colour, perView);
    g_debugManager->drawLine(ltf, ltb, colour, perView);
    g_debugManager->drawLine(lbb, ltb, colour, perView);
    g_debugManager->drawLine(rbb, rtb, colour, perView);
    g_debugManager->drawLine(rbf, rtf, colour, perView);
    g_debugManager->drawLine(lbf, ltf, colour, perView);
}
