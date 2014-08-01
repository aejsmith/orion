/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene view class.
 */

#include "gpu/gpu.h"

#include "render/scene.h"
#include "render/scene_view.h"

/** Initialize the scene view. */
SceneView::SceneView() :
	m_view_outdated(true),
	m_projection_outdated(true),
	m_uniforms_outdated(true)
{}

/** Destroy the scene view. */
SceneView::~SceneView() {}

/** Get the world-to-view matrix.
 * @return		World-to-view matrix. */
const glm::mat4 &SceneView::view() {
	if(m_view_outdated) {
		/* Viewing matrix is a world-to-view transformation, so we want
		 * the inverse of the given position and orientation. */
		glm::mat4 position = glm::translate(glm::mat4(), this->position);
		glm::mat4 orientation = glm::mat4_cast(glm::inverse(this->orientation));

		m_view = orientation * position;
		m_view_outdated = false;
	}

	return m_view;
}

/** Get the view-to-projection matrix.
 * @return		View-to-projection matrix. */
const glm::mat4 &SceneView::projection() {
	if(m_projection_outdated) {
		/* Determine the aspect ratio. */
		float aspect =
			static_cast<float>(this->viewport.width) /
			static_cast<float>(this->viewport.height);

		/* Convert horizontal field of view to vertical. */
		float fovx = glm::radians(this->fovx);
		float fovy = 2.0f * atanf(tanf(fovx * 0.5f) / aspect);

		m_projection = glm::perspective(fovy, aspect, this->znear, this->zfar);
		m_projection_outdated = false;
	}

	return m_projection;
}

/** Get the uniform buffer containing view parameters.
 * @return		Pointer to buffer containing view parameters. */
GPUBufferPtr SceneView::uniforms() {
	if(m_uniforms_outdated) {
		if(!m_uniforms) {
			/* Create the uniform buffer. */
			m_uniforms = g_engine->gpu()->create_buffer(
				GPUBuffer::kUniformBuffer,
				GPUBuffer::kDynamicDrawUsage,
				sizeof(ViewUniforms));
		}

		/* Ensure view and projection are up to date. */
		view();
		projection();
		glm::mat4 view_projection = m_projection * m_view;

		GPUBufferMapper<ViewUniforms> uniforms(m_uniforms, GPUBuffer::kMapInvalidate, GPUBuffer::kWriteAccess);

		memcpy(&uniforms->view, glm::value_ptr(m_view), sizeof(uniforms->view));
		memcpy(&uniforms->projection, glm::value_ptr(m_projection), sizeof(uniforms->projection));
		memcpy(&uniforms->view_projection, glm::value_ptr(view_projection), sizeof(uniforms->view_projection));
		memcpy(&uniforms->position, glm::value_ptr(this->position), sizeof(uniforms->position));

		m_uniforms_outdated = false;
	}

	return m_uniforms;
}
