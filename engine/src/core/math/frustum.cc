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
 * @brief               Frustum class.
 */

#include "core/math/frustum.h"

#include "engine/debug_manager.h"

/** Update the frustum based on the view-projection matrix.
 * @param matrix        View-projection matrix.
 * @param inverse       Inverse view-projection matrix. */
void Frustum::update(const glm::mat4 &matrix, const glm::mat4 &inverse) {
    /* http://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf */

    glm::vec4 planes[kNumPlanes];

    planes[kLeftPlane].x = matrix[0][3] + matrix[0][0];
    planes[kLeftPlane].y = matrix[1][3] + matrix[1][0];
    planes[kLeftPlane].z = matrix[2][3] + matrix[2][0];
    planes[kLeftPlane].w = -matrix[3][3] - matrix[3][0];

    planes[kRightPlane].x = matrix[0][3] - matrix[0][0];
    planes[kRightPlane].y = matrix[1][3] - matrix[1][0];
    planes[kRightPlane].z = matrix[2][3] - matrix[2][0];
    planes[kRightPlane].w = -matrix[3][3] + matrix[3][0];

    planes[kTopPlane].x = matrix[0][3] - matrix[0][1];
    planes[kTopPlane].y = matrix[1][3] - matrix[1][1];
    planes[kTopPlane].z = matrix[2][3] - matrix[2][1];
    planes[kTopPlane].w = -matrix[3][3] + matrix[3][1];

    planes[kBottomPlane].x = matrix[0][3] + matrix[0][1];
    planes[kBottomPlane].y = matrix[1][3] + matrix[1][1];
    planes[kBottomPlane].z = matrix[2][3] + matrix[2][1];
    planes[kBottomPlane].w = -matrix[3][3] - matrix[3][1];

    planes[kNearPlane].x = matrix[0][3] + matrix[0][2];
    planes[kNearPlane].y = matrix[1][3] + matrix[1][2];
    planes[kNearPlane].z = matrix[2][3] + matrix[2][2];
    planes[kNearPlane].w = -matrix[3][3] - matrix[3][2];

    planes[kFarPlane].x = matrix[0][3] - matrix[0][2];
    planes[kFarPlane].y = matrix[1][3] - matrix[1][2];
    planes[kFarPlane].z = matrix[2][3] - matrix[2][2];
    planes[kFarPlane].w = -matrix[3][3] + matrix[3][2];

    /* Normalize planes. */
    for (unsigned i = 0; i < kNumPlanes; i++) {
        const glm::vec4 &plane = planes[i];
        float magSquared = (plane.x * plane.x) + (plane.y * plane.y) + (plane.z * plane.z);
        m_planes[i] = planes[i] * glm::inversesqrt(magSquared);
    }

    /* Calculate corners. */
    static const glm::vec3 corners[kNumCorners] = {
        glm::vec3(-1.0f, 1.0f, -1.0f),
        glm::vec3(1.0f, 1.0f, -1.0f),
        glm::vec3(-1.0f, -1.0f, -1.0f),
        glm::vec3(1.0f, -1.0f, -1.0f),
        glm::vec3(-1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(-1.0f, -1.0f, 1.0f),
        glm::vec3(1.0f, -1.0f, 1.0f),
    };

    for (unsigned i = 0; i < kNumCorners; i++) {
        glm::vec4 inverted = inverse * glm::vec4(corners[i], 1.0f);
        m_corners[i] = glm::vec3(inverted) / inverted.w;
    }
}

/** Draw the frustum through the debug renderer.
 * @param colour        Colour to draw in.
 * @param perView       Whether to draw for the whole frame or just the next
 *                      view rendered. */
void Frustum::debugDraw(const glm::vec4 &colour, bool perView) const {
    const glm::vec3 *c = m_corners;
    g_debugManager->drawLine(c[kFarBottomLeftCorner], c[kFarBottomRightCorner], colour, perView);
    g_debugManager->drawLine(c[kFarBottomRightCorner], c[kNearBottomRightCorner], colour, perView);
    g_debugManager->drawLine(c[kNearBottomRightCorner], c[kNearBottomLeftCorner], colour, perView);
    g_debugManager->drawLine(c[kNearBottomLeftCorner], c[kFarBottomLeftCorner], colour, perView);
    g_debugManager->drawLine(c[kFarTopLeftCorner], c[kFarTopRightCorner], colour, perView);
    g_debugManager->drawLine(c[kFarTopRightCorner], c[kNearTopRightCorner], colour, perView);
    g_debugManager->drawLine(c[kNearTopRightCorner], c[kNearTopLeftCorner], colour, perView);
    g_debugManager->drawLine(c[kNearTopLeftCorner], c[kFarTopLeftCorner], colour, perView);
    g_debugManager->drawLine(c[kFarBottomLeftCorner], c[kFarTopLeftCorner], colour, perView);
    g_debugManager->drawLine(c[kFarBottomRightCorner], c[kFarTopRightCorner], colour, perView);
    g_debugManager->drawLine(c[kNearBottomRightCorner], c[kNearTopRightCorner], colour, perView);
    g_debugManager->drawLine(c[kNearBottomLeftCorner], c[kNearTopLeftCorner], colour, perView);
}
