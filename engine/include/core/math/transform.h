/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		3D transformation class.
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
	void set_position(const glm::vec3 &position);
	void set_orientation(const glm::quat &orientation);
	void set_scale(const glm::vec3 &scale);

	/** @return		Current position. */
	const glm::vec3 &position() const { return m_position; }
	/** @return		Current orientation. */
	const glm::quat &orientation() const { return m_orientation; }
	/** @return		Current scale. */
	const glm::vec3 &scale() const { return m_scale; }

	const glm::mat4 &matrix() const;
	glm::mat4 inverse_matrix() const;
private:
	glm::vec3 m_position;			/**< Position. */
	glm::quat m_orientation;		/**< Orientation. */
	glm::vec3 m_scale;			/**< Scale. */

	/** Precalculated matrices.
	 * @note		Mutable to allow updating cached versions in
	 *			const accessors. */
	mutable glm::mat4 m_matrix;		/**< Precalculated transformation matrix. */
	mutable bool m_matrix_outdated;		/**< Whether the matrix need updating. */
};

/** Initialize an identity transformation. */
inline Transform::Transform() :
	m_position(0.0f),
	m_orientation(1.0f, 0.0f, 0.0f, 0.0f),
	m_scale(1.0f),
	m_matrix(1.0f),
	m_matrix_outdated(false)
{}

/** Initialize a transformation.
 * @param position	Position.
 * @param orientation	Orientation.
 * @param scale		Scale. */
inline Transform::Transform(const glm::vec3 &position, const glm::quat &orientation, const glm::vec3 &scale) :
	m_position(position),
	m_orientation(orientation),
	m_scale(scale),
	m_matrix_outdated(true)
{}

/** Copy a transformation.
 * @param other		Transformation to copy. */
inline Transform::Transform(const Transform &other) :
	m_position(other.m_position),
	m_orientation(other.m_orientation),
	m_scale(other.m_scale),
	m_matrix(other.m_matrix),
	m_matrix_outdated(other.m_matrix_outdated)
{}

/** Assign a transformation.
 * @param other		Transformation to assign. */
inline Transform &Transform::operator =(const Transform &other) {
	m_position = other.m_position;
	m_orientation = other.m_orientation;
	m_scale = other.m_scale;
	m_matrix_outdated = other.m_matrix_outdated;
	if(!m_matrix_outdated)
		m_matrix = other.m_matrix;

	return *this;
}

/** Set the transformation.
 * @param position	New position.
 * @param orientation	New orientation.
 * @param scale		New scale. */
inline void Transform::set(const glm::vec3 &position, const glm::quat &orientation, const glm::vec3 &scale) {
	m_position = position;
	m_orientation = orientation;
	m_scale = scale;
	m_matrix_outdated = true;
}

/** Set the position.
 * @param position	New position. */
inline void Transform::set_position(const glm::vec3 &position) {
	m_position = position;
	m_matrix_outdated = true;
}

/** Set the orientation.
 * @param orientation	New orientation. */
inline void Transform::set_orientation(const glm::quat &orientation) {
	m_orientation = orientation;
	m_matrix_outdated = true;
}

/** Set the scale.
 * @param scale		New scale. */
inline void Transform::set_scale(const glm::vec3 &scale) {
	m_scale = scale;
	m_matrix_outdated = true;
}
