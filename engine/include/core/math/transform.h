/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               3D transformation class.
 */

#pragma once

#include "core/defs.h"

/**
 * Class encapsulating a 3D transformation.
 *
 * This class encapsulates a 3D object transformation (position, orientation
 * and scale), and calculates transformation matrices.
 */
class Transform {
public:
    Transform();
    Transform(const glm::vec3 &position, const glm::quat &orientation, const glm::vec3 &scale);
    Transform(const Transform &other);

    Transform &operator =(const Transform &other);

    void set(const glm::vec3 &position, const glm::quat &orientation, const glm::vec3 &scale);
    void setPosition(const glm::vec3 &position);
    void setOrientation(const glm::quat &orientation);
    void setScale(const glm::vec3 &scale);

    /** @return             Current position. */
    const glm::vec3 &position() const { return m_position; }
    /** @return             Current orientation. */
    const glm::quat &orientation() const { return m_orientation; }
    /** @return             Current scale. */
    const glm::vec3 &scale() const { return m_scale; }

    const glm::mat4 &matrix() const;
    glm::mat4 inverseMatrix() const;
private:
    glm::vec3 m_position;               /**< Position. */
    glm::quat m_orientation;            /**< Orientation. */
    glm::vec3 m_scale;                  /**< Scale. */

    /** Precalculated matrices. */
    mutable glm::mat4 m_matrix;         /**< Precalculated transformation matrix. */
    mutable bool m_matrixOutdated;      /**< Whether the matrix need updating. */
};

/** Initialize an identity transformation. */
inline Transform::Transform() :
    m_position(0.0f),
    m_orientation(1.0f, 0.0f, 0.0f, 0.0f),
    m_scale(1.0f),
    m_matrix(1.0f),
    m_matrixOutdated(false)
{}

/** Initialize a transformation.
 * @param position      Position.
 * @param orientation   Orientation.
 * @param scale         Scale. */
inline Transform::Transform(const glm::vec3 &position, const glm::quat &orientation, const glm::vec3 &scale) :
    m_position(position),
    m_orientation(orientation),
    m_scale(scale),
    m_matrixOutdated(true)
{}

/** Copy a transformation.
 * @param other         Transformation to copy. */
inline Transform::Transform(const Transform &other) :
    m_position(other.m_position),
    m_orientation(other.m_orientation),
    m_scale(other.m_scale),
    m_matrix(other.m_matrix),
    m_matrixOutdated(other.m_matrixOutdated)
{}

/** Assign a transformation.
 * @param other         Transformation to assign. */
inline Transform &Transform::operator =(const Transform &other) {
    m_position = other.m_position;
    m_orientation = other.m_orientation;
    m_scale = other.m_scale;
    m_matrixOutdated = other.m_matrixOutdated;
    if (!m_matrixOutdated)
        m_matrix = other.m_matrix;

    return *this;
}

/** Set the transformation.
 * @param position      New position.
 * @param orientation   New orientation.
 * @param scale         New scale. */
inline void Transform::set(const glm::vec3 &position, const glm::quat &orientation, const glm::vec3 &scale) {
    m_position = position;
    m_orientation = orientation;
    m_scale = scale;
    m_matrixOutdated = true;
}

/** Set the position.
 * @param position      New position. */
inline void Transform::setPosition(const glm::vec3 &position) {
    m_position = position;
    m_matrixOutdated = true;
}

/** Set the orientation.
 * @param orientation   New orientation. */
inline void Transform::setOrientation(const glm::quat &orientation) {
    m_orientation = orientation;
    m_matrixOutdated = true;
}

/** Set the scale.
 * @param scale         New scale. */
inline void Transform::setScale(const glm::vec3 &scale) {
    m_scale = scale;
    m_matrixOutdated = true;
}
