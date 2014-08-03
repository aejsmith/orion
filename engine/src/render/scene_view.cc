/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene view class.
 */

#include "gpu/gpu.h"

#include "render/scene.h"
#include "render/scene_view.h"

/**
 * Initialize the scene view.
 *
 * Initializes the scene view. The view is initially invalid: a transformation,
 * projection and viewport must be set manually.
 */
SceneView::SceneView() :
	m_view_outdated(true),
	m_projection_outdated(true),
	m_aspect(1.0f)
{}

/** Destroy the scene view. */
SceneView::~SceneView() {}

/** Set the viewing transformation.
 * @param position	Viewing position.
 * @param orientation	Viewing orientation. */
void SceneView::transform(const glm::vec3 &position, const glm::quat &orientation) {
	m_position = position;
	m_orientation = orientation;

	m_view_outdated = true;
	m_uniforms.invalidate();
}

/** Set a perspective projection.
 * @param fovx		Horizontal field of view, in degrees.
 * @param znear		Distance to near clipping plane.
 * @param zfar		Distance to far clipping plane. */
void SceneView::perspective(float fovx, float znear, float zfar) {
	m_fovx = fovx;
	m_znear = znear;
	m_zfar = zfar;

	m_projection_outdated = true;
	m_uniforms.invalidate();
}

/** Set the viewport.
 * @param viewport	Viewport rectangle in pixels. */
void SceneView::set_viewport(const IntRect &viewport) {
	m_viewport = viewport;

	/* Calculate aspect ratio. If it changes, we must recalculate the
	 * projection matrix. */
	float aspect = static_cast<float>(viewport.width) / static_cast<float>(viewport.height);
	if(aspect != m_aspect) {
		m_aspect = aspect;

		m_projection_outdated = true;
		m_uniforms.invalidate();
	}
}

/** Get the world-to-view matrix.
 * @return		World-to-view matrix. */
const glm::mat4 &SceneView::view() {
	if(m_view_outdated) {
		/* Viewing matrix is a world-to-view transformation, so we want
		 * the inverse of the given position and orientation. */
		glm::mat4 position = glm::translate(glm::mat4(), m_position);
		glm::mat4 orientation = glm::mat4_cast(glm::inverse(m_orientation));

		m_view = orientation * position;
		m_view_outdated = false;
	}

	return m_view;
}

/** Get the view-to-projection matrix.
 * @return		View-to-projection matrix. */
const glm::mat4 &SceneView::projection() {
	if(m_projection_outdated) {
		/* Convert horizontal field of view to vertical. */
		float fovx = glm::radians(m_fovx);
		float fovy = 2.0f * atanf(tanf(fovx * 0.5f) / m_aspect);

		m_projection = glm::perspective(fovy, m_aspect, m_znear, m_zfar);
		m_projection_outdated = false;
	}

	return m_projection;
}

/** Get the uniform buffer containing view parameters.
 * @return		Pointer to buffer containing view parameters. */
GPUBufferPtr SceneView::uniforms() {
	return m_uniforms.get([this](const GPUBufferMapper<ViewUniforms> &uniforms) {
		/* Ensure view and projection are up to date. */
		view();
		projection();

		/* Calculate combined view-projection matrix. */
		glm::mat4 view_projection = m_projection * m_view;

		memcpy(&uniforms->view, glm::value_ptr(m_view), sizeof(uniforms->view));
		memcpy(&uniforms->projection, glm::value_ptr(m_projection), sizeof(uniforms->projection));
		memcpy(&uniforms->view_projection, glm::value_ptr(view_projection), sizeof(uniforms->view_projection));
		memcpy(&uniforms->position, glm::value_ptr(m_position), sizeof(uniforms->position));
	});
}
