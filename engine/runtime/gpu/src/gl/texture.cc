/*
 * Copyright (C) 2015-2017 Alex Smith
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
 *
 * TODO:
 *  - Code paths for where we don't have ARB_texture_storage? May be needed for
 *    GLES or something.
 */

#include "gl.h"
#include "texture.h"
#include "window.h"

/** Initialize a new texture.
 * @param desc          Texture descriptor. */
GLTexture::GLTexture(const GPUTextureDesc &desc) :
    GPUTexture (desc),
    m_glTarget (GLUtil::convertTextureType(desc.type))
{
    glGenTextures(1, &m_texture);
    bindForModification();

    /* Specify maximum mipmap level. */
    glTexParameteri(m_glTarget, GL_TEXTURE_MAX_LEVEL, m_mips - 1);

    /* Specify storage for all levels. */
    switch (desc.type) {
        case kTexture2D:
        case kTextureCube:
            glTexStorage2D(m_glTarget,
                           m_mips,
                           g_opengl->pixelFormats[m_format].internalFormat,
                           m_width, m_height);
            break;
        case kTexture2DArray:
        case kTexture3D:
            glTexStorage3D(m_glTarget,
                           m_mips,
                           g_opengl->pixelFormats[m_format].internalFormat,
                           m_width, m_height, m_depth);
            break;
    }
}

/** Initialize a new texture view.
 * @param desc          Descriptor for the view. */
GLTexture::GLTexture(const GPUTextureViewDesc &desc) :
    GPUTexture (desc),
    m_glTarget (GLUtil::convertTextureType(m_type))
{
    GLTexture *source = static_cast<GLTexture *>(desc.source);

    unsigned layerCount;

    switch (m_type) {
        case kTexture2D:
        case kTexture3D:
            layerCount = 1;
            break;

        case kTexture2DArray:
            layerCount = m_depth;
            break;

        case kTextureCube:
            layerCount = CubeFace::kNumFaces;
            break;

    }

    glGenTextures(1, &m_texture);
    glTextureView(m_texture,
                  m_glTarget,
                  source->m_texture,
                  g_opengl->pixelFormats[m_format].internalFormat,
                  m_baseMip,
                  m_mips,
                  m_baseLayer,
                  layerCount);

    bindForModification();
    glTexParameteri(m_glTarget, GL_TEXTURE_MAX_LEVEL, m_mips - 1);
}

/** Initialize the texture as a dummy backing texture for the main window.
 * @param window        Main window. */
GLTexture::GLTexture(GLWindow *window) :
    GPUTexture (GPUTextureDesc().
                    setType   (GPUTexture::kTexture2D).
                    setWidth  (window->width()).
                    setHeight (window->height()).
                    setMips   (1).
                    setFlags  (GPUTexture::kRenderTarget).
                    setFormat (window->format())),
    m_texture  (0),
    m_glTarget (GL_TEXTURE_2D)
{}

/** Destroy the texture. */
GLTexture::~GLTexture() {
    if (m_texture != 0) {
        /* Invalidate all cached FBOs which refer to this texture. */
        g_opengl->invalidateFBOs(this);

        g_opengl->state.invalidateTexture(m_texture);
        glDeleteTextures(1, &m_texture);
    }
}

/** Bind the texture to a specific texture unit.
 * @param index         Texture unit index to bind to. */
void GLTexture::bind(unsigned index) {
    check(m_texture != 0);
    g_opengl->state.bindTexture(index, m_glTarget, m_texture);
}

/** Bind the texture for modification. */
void GLTexture::bindForModification() {
    check(m_texture != 0);

    /* We reserve the last available texture unit to bind textures to when
     * modifying them, rather than when using them for rendering. */
    g_opengl->state.bindTexture(g_opengl->features.maxTextureUnits - 1,
                                m_glTarget,
                                m_texture);
}

/** Update 2D texture area.
 * @param area          Area to update (2D rectangle).
 * @param data          Data to update with.
 * @param layer         Array layer/cube face.
 * @param mip           Mipmap level. */
void GLTexture::update(const IntRect &area, const void *data, unsigned mip, unsigned layer) {
    check(m_type == kTexture2D || m_type == kTexture2DArray || m_type == kTextureCube);
    check(mip < m_mips);
    check(layer < m_depth);

    if (!area.width || !area.height)
        return;

    bindForModification();

    if (m_type == kTexture2DArray) {
        glTexSubImage3D(m_glTarget, 
                        mip,
                        area.x, area.y, layer, area.width, area.height, 1,
                        g_opengl->pixelFormats[m_format].format,
                        g_opengl->pixelFormats[m_format].type,
                        data);
    } else {
        GLenum target = (m_type == kTextureCube)
                            ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer
                            : m_glTarget;

        glTexSubImage2D(target, 
                        mip,
                        area.x, area.y, area.width, area.height,
                        g_opengl->pixelFormats[m_format].format,
                        g_opengl->pixelFormats[m_format].type,
                        data);
    }
}

/** Update 3D texture area.
 * @param area          Area to update (3D box).
 * @param data          Data to update with.
 * @param mip           Mipmap level. */
void GLTexture::update(const IntBox &area, const void *data, unsigned mip) {
    check(m_type == kTexture3D);
    check(mip < m_mips);

    if (!area.width || !area.height || !area.depth)
        return;

    bindForModification();

    glTexSubImage3D(m_glTarget, 
                    mip,
                    area.x, area.y, area.z, area.width, area.height, area.depth,
                    g_opengl->pixelFormats[m_format].format,
                    g_opengl->pixelFormats[m_format].type,
                    data);
}

/** Generate mipmap images. */
void GLTexture::generateMipmap() {
    check(m_flags & kAutoMipmap);

    bindForModification();
    glGenerateMipmap(m_glTarget);
}

/**
 * Texture creation methods.
 */

/** Create a texture.
 * @param desc          Descriptor containing texture parameters.
 * @return              Pointer to created texture. */
GPUTexturePtr GLGPUManager::createTexture(const GPUTextureDesc &desc) {
    return new GLTexture(desc);
}

/** Create a texture view.
 * @param desc          Descriptor for the view.
 * @return              Pointer to created texture view. */
GPUTexturePtr GLGPUManager::createTextureView(const GPUTextureViewDesc &desc) {
    return new GLTexture(desc);
}
