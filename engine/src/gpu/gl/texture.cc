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

/** Common texture initialization.
 * @param desc          Texture descriptor.
 * @param target        Target for the texture. */
template <typename Desc>
GLTexture::GLTexture(const Desc &desc, GLenum target) :
    GPUTexture(desc),
    m_glTarget(target)
{
    /* Create the texture and bind it for the per-type constructor to use. */
    glGenTextures(1, &m_texture);
    bindForModification();

    /* Specify maximum mipmap level. */
    glTexParameteri(m_glTarget, GL_TEXTURE_MAX_LEVEL, m_mips);
}

/** Initialize the texture as a 2D texture.
 * @param desc          Descriptor containing texture parameters. */
GLTexture::GLTexture(const GPUTexture2DDesc &desc) :
    GLTexture(desc, GL_TEXTURE_2D)
{
    /* Specify storage for all levels. */
    glTexStorage2D(
        GL_TEXTURE_2D,
        m_mips,
        g_opengl->pixelFormats[m_format].internalFormat,
        m_width, m_height);
}

/** Initialize the texture as a 2D array texture.
 * @param desc          Descriptor containing texture parameters. */
GLTexture::GLTexture(const GPUTexture2DArrayDesc &desc) :
    GLTexture(desc, GL_TEXTURE_2D_ARRAY)
{
    glTexStorage3D(
        GL_TEXTURE_2D_ARRAY,
        m_mips,
        g_opengl->pixelFormats[m_format].internalFormat,
        m_width, m_height, m_depth);
}

/** Initialize the texture as a cube texture.
 * @param desc          Descriptor containing texture parameters. */
GLTexture::GLTexture(const GPUTextureCubeDesc &desc) :
    GLTexture(desc, GL_TEXTURE_CUBE_MAP)
{
    glTexStorage2D(
        GL_TEXTURE_CUBE_MAP,
        m_mips,
        g_opengl->pixelFormats[m_format].internalFormat,
        m_width, m_height);
}

/** Initialize the texture as a 3D texture.
 * @param desc          Descriptor containing texture parameters. */
GLTexture::GLTexture(const GPUTexture3DDesc &desc) :
    GLTexture(desc, GL_TEXTURE_3D)
{
    glTexStorage3D(
        GL_TEXTURE_3D,
        m_mips,
        g_opengl->pixelFormats[m_format].internalFormat,
        m_width, m_height, m_depth);
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

/** Create a 2D texture.
 * @param desc          Descriptor containing texture parameters.
 * @return              Pointer to created texture. */
GPUTexturePtr GLGPUManager::createTexture(const GPUTexture2DDesc &desc) {
    return new GLTexture(desc);
}

/** Create a 2D array texture.
 * @param desc          Descriptor containing texture parameters.
 * @return              Pointer to created texture. */
GPUTexturePtr GLGPUManager::createTexture(const GPUTexture2DArrayDesc &desc) {
    return new GLTexture(desc);
}

/** Create a cube texture.
 * @param desc          Descriptor containing texture parameters.
 * @return              Pointer to created texture. */
GPUTexturePtr GLGPUManager::createTexture(const GPUTextureCubeDesc &desc) {
    return new GLTexture(desc);
}

/** Create a 3D texture.
 * @param desc          Descriptor containing texture parameters.
 * @return              Pointer to created texture. */
GPUTexturePtr GLGPUManager::createTexture(const GPUTexture3DDesc &desc) {
    return new GLTexture(desc);
}
