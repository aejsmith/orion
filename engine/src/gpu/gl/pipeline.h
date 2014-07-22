/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL pipeline implementation.
 */

#ifndef ORION_GPU_GL_PIPELINE_H
#define ORION_GPU_GL_PIPELINE_H

#include "defs.h"

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
	void _finalize();
private:
	GLuint m_pipeline;		/**< Pipeline object ID. */
};

#endif /* ORION_GPU_GL_PIPELINE_H */
