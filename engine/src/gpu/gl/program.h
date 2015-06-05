/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               OpenGL program implementation.
 */

#pragma once

#include "gl.h"

/** OpenGL GPU shader implementation. */
class GLProgram : public GPUProgram {
public:
    GLProgram(unsigned stage, GLuint program);
    ~GLProgram();

    void queryUniformBlocks(ResourceList &list) override;
    void querySamplers(ResourceList &list) override;
    void bindUniformBlock(unsigned index, unsigned slot) override;
    void bindSampler(unsigned index, unsigned slot) override;

    /** Get the GL program object.
     * @return              GL program object ID. */
    GLuint program() const { return m_program; }
private:
    GLuint m_program;               /**< Program object ID. */
};
