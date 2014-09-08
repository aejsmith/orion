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
	m_viewOutdated(true),
	m_projectionOutdated(true),
	m_aspect(1.0f)
{}

/** Destroy the scene view. */
SceneView::~SceneView() {}

/** Set the viewing transformation.
 * @param position	Viewing position.
 * @param orientation	Viewing orientation. */
void SceneView::setTransform(const glm::vec3 &position, const glm::quat &orientation) {
	m_position = position;
	m_orientation = orientation;

	m_viewOutdated = true;
	m_uniforms.invalidate();
}

/** Set a perspective projection.
 * @param fov		Horizontal field of view, in degrees.
 * @param zNear		Distance to near clipping plane.
 * @param zFar		Distance to far clipping plane. */
void SceneView::perspective(float fov, float zNear, float zFar) {
	m_fov = fov;
	m_zNear = zNear;
	m_zFar = zFar;

	m_projectionOutdated = true;
	m_uniforms.invalidate();
}

/** Set the viewport.
 * @param viewport	Viewport rectangle in pixels. */
void SceneView::setViewport(const IntRect &viewport) {
	m_viewport = viewport;

	/* Calculate aspect ratio. If it changes, we must recalculate the
	 * projection matrix. */
	float aspect = static_cast<float>(viewport.width) / static_cast<float>(viewport.height);
	if(aspect != m_aspect) {
		m_aspect = aspect;

		m_projectionOutdated = true;
		m_uniforms.invalidate();
	}
}

/** Get the world-to-view matrix.
 * @return		World-to-view matrix. */
const glm::mat4 &SceneView::view() {
	if(m_viewOutdated) {
		/* Viewing matrix is a world-to-view transformation, so we want
		 * the inverse of the given position and orientation. */
		glm::mat4 position = glm::translate(glm::mat4(), -m_position);
		glm::mat4 orientation = glm::mat4_cast(glm::inverse(m_orientation));

		m_view = orientation * position;
		m_viewOutdated = false;
	}

	return m_view;
}

/** Get the view-to-projection matrix.
 * @return		View-to-projection matrix. */
const glm::mat4 &SceneView::projection() {
	if(m_projectionOutdated) {
		/* Convert horizontal field of view to vertical. */
		float fov = glm::radians(m_fov);
		float verticalFOV = 2.0f * atanf(tanf(fov * 0.5f) / m_aspect);

		m_projection = glm::perspective(verticalFOV, m_aspect, m_zNear, m_zFar);
		m_projectionOutdated = false;
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
		glm::mat4 viewProjection = m_projection * m_view;

		memcpy(&uniforms->view, glm::value_ptr(m_view), sizeof(uniforms->view));
		memcpy(&uniforms->projection, glm::value_ptr(m_projection), sizeof(uniforms->projection));
		memcpy(&uniforms->viewProjection, glm::value_ptr(viewProjection), sizeof(uniforms->viewProjection));
		memcpy(&uniforms->position, glm::value_ptr(m_position), sizeof(uniforms->position));
	});
}
