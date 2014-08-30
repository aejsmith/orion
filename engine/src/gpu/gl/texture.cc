/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GL texture implementation.
 *
 * @todo		Code paths for where we don't have ARB_texture_storage?
 *			May be needed for GLES or something.
 */

#include "gl.h"
#include "texture.h"

/** Common texture initialization.
 * @param desc		Texture descriptor.
 * @param target	Target for the texture. */
template <typename Desc>
GLTexture::GLTexture(const Desc &desc, GLenum target) :
	GPUTexture(desc),
	m_gl_target(target)
{
	/* Create the texture and bind it for the per-type constructor to use. */
	glGenTextures(1, &m_texture);
	bind_for_modification();

	/* Specify maximum mipmap level. */
	glTexParameteri(m_gl_target, GL_TEXTURE_MAX_LEVEL, m_mips);
}

/** Initialize the texture as a 2D texture.
 * @param desc		Descriptor containing texture parameters. */
GLTexture::GLTexture(const GPUTexture2DDesc &desc) :
	GLTexture(desc, GL_TEXTURE_2D)
{
	/* Specify storage for all levels. */
	glTexStorage2D(
		GL_TEXTURE_2D,
		m_mips,
		g_opengl->pixel_formats[m_format].internal_format,
		m_width, m_height);
}

/** Initialize the texture as a 2D array texture.
 * @param desc		Descriptor containing texture parameters. */
GLTexture::GLTexture(const GPUTexture2DArrayDesc &desc) :
	GLTexture(desc, GL_TEXTURE_2D_ARRAY)
{
	glTexStorage3D(
		GL_TEXTURE_2D_ARRAY,
		m_mips,
		g_opengl->pixel_formats[m_format].internal_format,
		m_width, m_height, m_depth);
}

/** Initialize the texture as a cube texture.
 * @param desc		Descriptor containing texture parameters. */
GLTexture::GLTexture(const GPUTextureCubeDesc &desc) :
	GLTexture(desc, GL_TEXTURE_CUBE_MAP)
{
	glTexStorage2D(
		GL_TEXTURE_CUBE_MAP,
		m_mips,
		g_opengl->pixel_formats[m_format].internal_format,
		m_width, m_height);
}

/** Initialize the texture as a 3D texture.
 * @param desc		Descriptor containing texture parameters. */
GLTexture::GLTexture(const GPUTexture3DDesc &desc) :
	GLTexture(desc, GL_TEXTURE_3D)
{
	glTexStorage3D(
		GL_TEXTURE_3D,
		m_mips,
		g_opengl->pixel_formats[m_format].internal_format,
		m_width, m_height, m_depth);
}

/** Destroy the texture. */
GLTexture::~GLTexture() {
	glDeleteTextures(1, &m_texture);
}

/** Bind the texture for modification. */
void GLTexture::bind_for_modification() {
	/* We reserve the last available texture unit to bind textures to when
	 * modifying them, rather than when using them for rendering. */
	g_opengl->state.bind_texture(
		g_opengl->features.max_texture_units - 1,
		m_gl_target,
		m_texture);
}

/** Update 2D texture area.
 * @param area		Area to update (2D rectangle).
 * @param data		Data to update with.
 * @param layer		Array layer/cube face.
 * @param mip		Mipmap level. */
void GLTexture::update(const Rect &area, const void *data, unsigned mip, unsigned layer) {
	orion_assert(m_type == kTexture2D || m_type == kTexture2DArray || m_type == kTextureCube);
	orion_assert(mip < m_mips);
	orion_assert(layer < m_depth);

	bind_for_modification();

	if(m_type == kTexture2DArray) {
		glTexSubImage3D(
			m_gl_target, 
			mip,
			area.x, area.y, layer, area.width, area.height, 1,
			g_opengl->pixel_formats[m_format].format,
			g_opengl->pixel_formats[m_format].type,
			data);
	} else {
		GLenum target = (m_type == kTextureCube)
			? GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer
			: m_gl_target;

		glTexSubImage2D(
			target, 
			mip,
			area.x, area.y, area.width, area.height,
			g_opengl->pixel_formats[m_format].format,
			g_opengl->pixel_formats[m_format].type,
			data);
	}
}

/** Update 3D texture area.
 * @param area		Area to update (3D box).
 * @param data		Data to update with.
 * @param mip		Mipmap level. */
void GLTexture::update(const Box &area, const void *data, unsigned mip) {
	orion_assert(m_type == kTexture3D);
	orion_assert(mip < m_mips);

	bind_for_modification();

	glTexSubImage3D(
		m_gl_target, 
		mip,
		area.x, area.y, area.z, area.width, area.height, area.depth,
		g_opengl->pixel_formats[m_format].format,
		g_opengl->pixel_formats[m_format].type,
		data);
}

/** Generate mipmap images. */
void GLTexture::generate_mips() {
	orion_assert(m_flags & kAutoMipmap);

	bind_for_modification();
	glGenerateMipmap(m_gl_target);
}

/**
 * Texture creation methods.
 */

/** Create a 2D texture.
 * @param desc		Descriptor containing texture parameters.
 * @return		Pointer to created texture. */
GPUTexturePtr GLGPUInterface::create_texture(const GPUTexture2DDesc &desc) {
	GPUTexture *texture = new GLTexture(desc);
	return GPUTexturePtr(texture);
}

/** Create a 2D array texture.
 * @param desc		Descriptor containing texture parameters.
 * @return		Pointer to created texture. */
GPUTexturePtr GLGPUInterface::create_texture(const GPUTexture2DArrayDesc &desc) {
	GPUTexture *texture = new GLTexture(desc);
	return GPUTexturePtr(texture);
}

/** Create a cube texture.
 * @param desc		Descriptor containing texture parameters.
 * @return		Pointer to created texture. */
GPUTexturePtr GLGPUInterface::create_texture(const GPUTextureCubeDesc &desc) {
	GPUTexture *texture = new GLTexture(desc);
	return GPUTexturePtr(texture);
}

/** Create a 3D texture.
 * @param desc		Descriptor containing texture parameters.
 * @return		Pointer to created texture. */
GPUTexturePtr GLGPUInterface::create_texture(const GPUTexture3DDesc &desc) {
	GPUTexture *texture = new GLTexture(desc);
	return GPUTexturePtr(texture);
}
