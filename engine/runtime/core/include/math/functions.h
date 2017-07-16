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
 * @brief               Math utility functions.
 */

#pragma once

#include "core/defs.h"

namespace Math {
    /** Round a value up.
     * @param val           Value to round.
     * @param nearest       Boundary to round up to.
     * @return              Rounded value. */
    template <typename T, typename U>
    inline constexpr T roundUp(const T &val, const U &nearest) {
        /* When nearest is a power of 2, this is optimised to be equivalent to
         * the following:
         *  if (val & (nearest - 1)) {
         *      val += nearest;
         *      val &= ~(nearest - 1);
         *  }
         * Using the implementation below has the benefit that we do not limit
         * use to power-of-2 alignment. */
        return (val % nearest) ? (val - (val % nearest)) + nearest : val;
    }

    /** Round a value down.
     * @param val           Value to round.
     * @param nearest       Boundary to round down to.
     * @return              Rounded value. */
    template <typename T, typename U>
    inline constexpr T roundDown(const T &val, const U &nearest) {
        /* Same as above. */
        return (val % nearest) ? val - (val % nearest) : val;
    }

    /** Check if a value is a power of 2.
     * @param val           Value to check.
     * @return              Whether value is a power of 2. */
    template <typename T>
    inline constexpr bool isPow2(const T &val) {
        return val && (val & (val - 1)) == 0;
    }

    /**
     * Compute a quaternion which rotates from one vector to another.
     *
     * Computes a rotation quaternion which when applied to the given from
     * vector will yield the given to vector.
     *
     * @param from          Vector to rotate from.
     * @param to            Vector to rotate to.
     *
     * @return              Quaternion which will rotate between the two vectors.
     */
    static inline glm::quat quatRotateBetween(const glm::vec3 &from, const glm::vec3 &to) {
        /* Based on code from
         * http://lolengine.net/blog/2014/02/24/quaternion-from-two-vectors-final */

        float fromToLength = glm::sqrt(glm::dot(from, from) * glm::dot(to, to));
        float w = fromToLength + glm::dot(from, to);
        glm::vec3 n;

        if (w < 1.0e-6f * fromToLength) {
            w = 0.0f;
            n = (glm::abs(from.x) > glm::abs(from.z))
                    ? glm::vec3(-from.y, from.x, 0.0f)
                    : glm::vec3(0.0f, -from.z, from.y);
        } else {
            n = glm::cross(from, to);
        }

        return glm::normalize(glm::quat(w, n));
    }

    /**
     * Compute a quaternion orientation which looks at a given point.
     *
     * Generates a quaternion which rotates the forward axis (-Z) to be aligned
     * along the specified forward direction, and the up axis (+Y) to be aligned
     * along the orthonormalisation of the specified up direction according to
     * the forward direction.
     *
     * @param forward       Forward direction.
     * @param up            Up direction (guideline).
     *
     * @return              Rotation quaternion.
     */
    static inline glm::quat quatLookAt(const glm::vec3 &forward,
                                       const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        glm::mat4 look = glm::lookAt(glm::vec3(0.0), forward, up);
        return glm::inverse(glm::normalize(glm::quat_cast(look)));
    }

    /**
     * Compute the difference between quaternion orientations.
     *
     * Given two quaternions, a and b, returns the quaternion d such that
     * d * a = b.
     *
     * @param a             Source orientation.
     * @param b             Destination orientation.
     *
     * @return              Orientation difference.
     */
    static inline glm::quat quatDifference(const glm::quat &a, const glm::quat &b) {
        return b * glm::inverse(a);
    }
}
