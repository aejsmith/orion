/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               OpenGL pipeline implementation.
 */

#pragma once

#include "gl.h"

/** OpenGL pipeline implementation. */
class GLPipeline : public GPUPipeline {
public:
    explicit GLPipeline(const GPUShaderArray &shaders);
    ~GLPipeline();

    void bind();

    /** Get the pipeline object ID.
     * @return              Pipeline object ID. */
    GLuint pipeline() const { return m_pipeline; }
private:
    GLuint m_pipeline;              /**< Pipeline object ID. */
};
