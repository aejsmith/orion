/*
 * Copyright (C) 2015 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
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
