/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               GL texture implementation.
 *
 * @todo                Code paths for where we don't have ARB_texture_storage?
 *                      May be needed for GLES or something.
 */

#include "gl.h"
#include "texture.h"

/** Initialize a new texture.
 * @param desc          Texture descriptor. */
GLTexture::GLTexture(const GPUTextureDesc &desc) :
    GPUTexture(desc),
    m_glTarget(GLUtil::convertTextureType(desc.type))
{
    /* Create the texture and bind it for the per-type constructor to use. */
    glGenTextures(1, &m_texture);
    bindForModification();

    /* Specify maximum mipmap level. */
    glTexParameteri(m_glTarget, GL_TEXTURE_MAX_LEVEL, m_mips);

    /* Specify storage for all levels. */
    switch (desc.type) {
        case kTexture2D:
        case kTextureCube:
            glTexStorage2D(
                m_glTarget,
                m_mips,
                g_opengl->pixelFormats[m_format].internalFormat,
                m_width, m_height);
            break;
        case kTexture2DArray:
        case kTexture3D:
            glTexStorage3D(
                m_glTarget,
                m_mips,
                g_opengl->pixelFormats[m_format].internalFormat,
                m_width, m_height, m_depth);
            break;
    }
}

/** Destroy the texture. */
GLTexture::~GLTexture() {
    /* Invalidate all cached FBOs which refer to this texture. */
    g_opengl->invalidateFBOs(this);

    g_opengl->state.invalidateTexture(m_texture);
    glDeleteTextures(1, &m_texture);
}

/** Bind the texture to a specific texture unit.
 * @param index         Texture unit index to bind to. */
void GLTexture::bind(unsigned index) {
    g_opengl->state.bindTexture(index, m_glTarget, m_texture);
}

/** Bind the texture for modification. */
void GLTexture::bindForModification() {
    /* We reserve the last available texture unit to bind textures to when
     * modifying them, rather than when using them for rendering. */
    g_opengl->state.bindTexture(
        g_opengl->features.maxTextureUnits - 1,
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

    bindForModification();

    if (m_type == kTexture2DArray) {
        glTexSubImage3D(
            m_glTarget, 
            mip,
            area.x, area.y, layer, area.width, area.height, 1,
            g_opengl->pixelFormats[m_format].format,
            g_opengl->pixelFormats[m_format].type,
            data);
    } else {
        GLenum target = (m_type == kTextureCube)
            ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer
            : m_glTarget;

        glTexSubImage2D(
            target, 
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

    bindForModification();

    glTexSubImage3D(
        m_glTarget, 
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