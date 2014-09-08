/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL pipeline implementation.
 */

#pragma once

#include "gl.h"

/** OpenGL pipeline implementation. */
class GLPipeline : public GPUPipeline {
public:
	GLPipeline();
	~GLPipeline();

	void bind();

	/** Get the pipeline object ID.
	 * @return		Pipeline object ID. */
	GLuint pipeline() const { return m_pipeline; }
protected:
	void finalizeImpl() override;
private:
	GLuint m_pipeline;		/**< Pipeline object ID. */
};
