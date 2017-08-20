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
 * @brief               GL texture implementation.
 */

#pragma once

#include "gl.h"

#include "gpu/texture.h"

class GLWindow;

/** OpenGL texture implementation. */
class GLTexture : public GPUTexture {
public:
    explicit GLTexture(const GPUTextureDesc &desc);
    explicit GLTexture(const GPUTextureViewDesc &desc);
    explicit GLTexture(GLWindow *window);

    ~GLTexture();

    void update(const IntRect &area, const void *data, unsigned mip, unsigned layer) override;
    void update(const IntBox &area, const void *data, unsigned mip) override;
    void generateMipmap() override;

    void bind(unsigned index);

    /** @return             Whether this texture is a dummy backing texture for
     *                      the main window. */
    bool isMainWindow() const { return m_texture == 0; }

    /** @return             GL texture ID. */
    GLuint texture() const { return m_texture; }
    /** @return             GL target. */
    GLenum glTarget() const { return m_glTarget; }
private:
    void bindForModification();
private:
    GLuint m_texture;               /**< GL texture handle. */
    GLenum m_glTarget;              /**< Target for the texture. */
};
