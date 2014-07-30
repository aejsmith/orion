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
 * Initialize the scene view. The view will initially have invalid view and
 * projection matrices, they must be set manually. The viewport rectangle will
 * initially be set to (0, 0, 1, 1).
 *
 * @param scene		Scene that the view is for.
 */
SceneView::SceneView(Scene *scene) :
	m_scene(scene),
	m_position(0.0f, 0.0f, 0.0f),
	m_orientation(1.0f, 0.0f, 0.0f, 0.0f),
	m_uniforms_outdated(true)
{}

/** Destroy the scene view. */
SceneView::~SceneView() {}

/** Set the world transformation of the view.
 * @param position	Position of the view.
 * @param orientation	Orientation of the view. */
void SceneView::set_transform(const glm::vec3 &position, const glm::quat &orientation) {
	m_position = position;
	m_orientation = orientation;

	/* Viewing matrix is a world-to-view transformation, so we want the
	 * inverse of the given position and orientation. */
	m_view = glm::mat4_cast(glm::inverse(orientation)) * glm::translate(glm::mat4(), position);

	m_uniforms_outdated = true;
}

/** Set the projection matrix.
 * @param projection	New projection matrix. */
void SceneView::set_projection(const glm::mat4 &projection) {
	m_projection = projection;
	m_uniforms_outdated = true;
}

/**
 * Get the uniform buffer containing view parameters.
 *
 * Gets the uniform buffer which contains per-view parameters. This function
 * will update the buffer with the latest parameters if it is currently out of
 * date.
 *
 * @return		Pointer to buffer containing view parameters.
 */
GPUBufferPtr SceneView::uniforms() {
	if(m_uniforms_outdated) {
		if(!m_uniforms) {
			/* Create the uniform buffer. */
			m_uniforms = g_gpu->create_buffer(GPUBuffer::kUniformBuffer,
				GPUBuffer::kDynamicDrawUsage,
				sizeof(ViewUniforms));
		}

		glm::mat4 view_projection = m_projection * m_view;

		GPUBufferMapper<ViewUniforms> uniforms(m_uniforms,
			GPUBuffer::kMapInvalidate,
			GPUBuffer::kWriteAccess);

		memcpy(&uniforms->view, glm::value_ptr(m_view), sizeof(uniforms->view));
		memcpy(&uniforms->projection, glm::value_ptr(m_projection), sizeof(uniforms->projection));
		memcpy(&uniforms->view_projection, glm::value_ptr(view_projection), sizeof(uniforms->view_projection));
		memcpy(&uniforms->position, glm::value_ptr(m_position), sizeof(uniforms->position));

		m_uniforms_outdated = false;
	}

	return m_uniforms;
}
