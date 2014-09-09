/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Texture asset classes.
 */

#include "engine/texture.h"

#include "gpu/gpu.h"

/**
 * Create a 2D texture.
 *
 * Creates a new 2D texture. The default arguments give the texture a format of
 * PixelFormat::kR8G8B8A8, and a full mipmap pyramid which can be automatically
 * updated.
 *
 * @param width		Width of the texture.
 * @param height	Height of the texture.
 * @param format	Pixel format to use.
 * @param mips		Number of mip levels (0 for full pyramid).
 * @param flags		GPU texture creation flags.
 */
Texture2D::Texture2D(uint32_t width, uint32_t height, PixelFormat format, unsigned mips, uint32_t flags) {
	GPUTexture2DDesc desc;
	desc.width = width;
	desc.height = height;
	desc.format = format;
	desc.mips = mips;
	desc.flags = flags;

	m_gpu = g_gpu->createTexture(desc);
}

/**
 * Replace the entire texture content.
 *
 * Replaces the entire content of the top mip level of the texture with the
 * contents of the supplied buffer. The buffer must contain pixel data in the
 * same format as the texture, and it must equal the size of the texture.
 *
 * If updateMipmap is true (the default), the mipmap images of the texture
 * will be regenerated based on the new image content. Note that this will only
 * actually be done if the texture was created with the GPUTexture::kAutoMipmap
 * flag set.
 *
 * @param data		New texture data.
 * @param updateMipmap	Whether to update mipmap images (defaults to true).
 */
void Texture2D::update(const void *data, bool updateMipmap) {
	Rect area(0, 0, m_gpu->width(), m_gpu->height());
	m_gpu->update(area, data);

	/* Regenerate mipmaps if requested. */
	if(updateMipmap && m_gpu->flags() & GPUTexture::kAutoMipmap)
		m_gpu->generateMipmap();
}

/**
 * Update an area of the texture.
 *
 * Updates an area of the top mip level of the texture. The buffer must contain
 * pixel data in the same format as the texture, and it must equal the area
 * size specified.
 *
 * If updateMipmap is true (the default), the mipmap images of the texture
 * will be regenerated based on the updated image content. Note that this will
 * only actually be done if the texture was created with the
 * GPUTexture::kAutoMipmap flag set.
 *
 * @param area		Area to update.
 * @param data		New texture data.
 * @param updateMipmap	Whether to update mipmap images (defaults to true).
 */
void Texture2D::update(const Rect &area, const void *data, bool updateMipmap) {
	m_gpu->update(area, data);

	/* Regenerate mipmaps if requested. */
	if(updateMipmap && m_gpu->flags() & GPUTexture::kAutoMipmap)
		m_gpu->generateMipmap();
}

/**
 * Update a specific mip level of the texture.
 *
 * Updates an area of a specific mip level of the texture. The buffer must
 * contain pixel data in the same format as the texture, and it must equal the
 * area size specified. No mipmap regeneration will be performed.
 *
 * @param mip		Mip level to update.
 * @param area		Area to update.
 * @param data		New texture data.
 */
void Texture2D::update(unsigned mip, const Rect &area, const void *data) {
	orionAssert(mip < mips());

	m_gpu->update(area, data, mip);
}
