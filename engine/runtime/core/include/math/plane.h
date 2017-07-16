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
 * @brief               Plane class.
 */

#pragma once

#include "core/defs.h"

/**
 * Class representing a plane.
 *
 * This class represents a plane in 3D space as a normal vector plus a distance
 * from the origin to the plane. The side away from which the normal points is
 * the positive half-space. The distance from the origin is in the direction
 * of the normal.
 */
class Plane {
public:
    /** Construct as an invalid plane. */
    Plane() {}

    /** Construct from existing plane vector.
     * @param vector        Plane vector. */
    Plane(const glm::vec4 &vector) :
        m_vector (vector)
    {}

    /** Construct from normal and distance.
     * @param normal        Plane normal (must be normalised).
     * @param distance      Distance from origin. */
    Plane(const glm::vec3 &normal, float distance) :
        m_vector (normal, distance)
    {}

    /** Construct from a normal and a known point on the plane.
     * @param normal        Plane normal (must be normalised).
     * @param point         Known point on the plane. */
    Plane(const glm::vec3 &normal, const glm::vec3 &point) :
        m_vector (normal, glm::dot(normal, point))
    {}

    /** @return             Vector representation of plane. */
    const glm::vec4 &vector() const { return m_vector; }
    /** @return             Normal of plane. */
    glm::vec3 normal() const { return glm::vec3(m_vector); }
    /** @return             Distance from origin to plane in normal direction. */
    float distance() const { return m_vector.w; }

    /** Get the distance to a point from the plane.
     * @param point         Point to check.
     * @return              Signed distance to the point, positive if in front
     *                      of the plane, i.e. in the direction of the normal. */
    float distanceTo(const glm::vec3 &point) const {
        return glm::dot(glm::vec3(m_vector), point) - m_vector.w;
    }
private:
    /** Vector representation (normal + distance). */
    glm::vec4 m_vector;
};
