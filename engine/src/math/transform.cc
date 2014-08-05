/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		3D transformation class.
 */

#include "math/transform.h"

#include <glm/gtc/matrix_inverse.hpp>

/** Get the transformation matrix.
 * @return		Transformation matrix for the transformation. */
const glm::mat4 &Transform::matrix() const {
	if(m_matrix_outdated) {
		m_matrix =
			glm::translate(glm::mat4(), m_position) *
			glm::mat4_cast(m_orientation) *
			glm::scale(glm::mat4(), m_scale);
	}

	return m_matrix;
}

/** Get the inverse transformation matrix.
 * @return		Inverse transformation matrix for the transformation. */
glm::mat4 Transform::inverse_matrix() const {
	return glm::affineInverse(matrix());
}
