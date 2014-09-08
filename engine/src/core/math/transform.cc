/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		3D transformation class.
 */

#include "core/math/transform.h"

/** Get the transformation matrix.
 * @return		Transformation matrix for the transformation. */
const glm::mat4 &Transform::matrix() const {
	if(m_matrixOutdated) {
		m_matrix =
			glm::translate(glm::mat4(), m_position) *
			glm::mat4_cast(m_orientation) *
			glm::scale(glm::mat4(), m_scale);
		m_matrixOutdated = false;
	}

	return m_matrix;
}

/** Get the inverse transformation matrix.
 * @return		Inverse transformation matrix for the transformation. */
glm::mat4 Transform::inverseMatrix() const {
	return glm::affineInverse(matrix());
}
