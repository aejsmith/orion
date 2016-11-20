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

#pragma once

#include "core/math/plane.h"

/**
 * Class representing a frustum.
 *
 * This class encorporates some utility functionality for frustums. It does not
 * include functionality for defining a frustum and its matrices (this is left
 * to Camera/SceneView), rather it takes pre-calculated view/projection matrices
 * and converts them to a plane representation in order to peform intersection
 * tests, etc. Note that the positive half-space of each plane is inside the
 * frustum.
 */
class Frustum {
public:
    /** Frustum plane enumeration. */
    enum {
        kLeftPlane,
        kRightPlane,
        kTopPlane,
        kBottomPlane,
        kNearPlane,
        kFarPlane,
        kNumPlanes,
    };

    /** Frustum corner enumeration. */
    enum {
        kNearTopLeftCorner,
        kNearTopRightCorner,
        kNearBottomLeftCorner,
        kNearBottomRightCorner,
        kFarTopLeftCorner,
        kFarTopRightCorner,
        kFarBottomLeftCorner,
        kFarBottomRightCorner,
        kNumCorners,
    };
public:
    /** Initialise as an invalid frustum. */
    Frustum() {}

    /** Initialise from combined view-projection matrices.
     * @param matrix        View-projection matrix.
     * @param inverse   Inverse view-projection matrix. */
    Frustum(const glm::mat4 &matrix, const glm::mat4 &inverse) {
        update(matrix, inverse);
    }

    /** Get a plane of the frustum.
     * @param plane         Index of plane to get.
     * @return              Selected plane of the frustum. */
    const Plane &plane(unsigned plane) const {
        return m_planes[plane];
    }

    /** Get a corner of the frustum.
     * @param corner        Corner of frustum to get.
     * @return              Selected plane of the frustum. */
    const glm::vec3 &corner(unsigned corner) const {
        return m_corners[corner];
    }

    void update(const glm::mat4 &matrix, const glm::mat4 &inverse);
private:
    /** Planes of the frustum. */
    Plane m_planes[kNumPlanes];

    /** Corners of the frustum. */
    glm::vec3 m_corners[kNumCorners];
};
