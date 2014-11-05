/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Texture asset classes.
 */

#include "engine/texture.h"

#include "gpu/gpu.h"

/** Private constructor, does not actually create the texture. */
TextureBase::TextureBase() :
	m_filterMode(SamplerFilterMode::kAnisotropic),
	m_anisotropy(8),
	m_addressMode(SamplerAddressMode::kClamp)
{
	updateSamplerState();
}

/**
 * Set the texture filtering mode.
 *
 * Sets the texture filtering mode for this texture. By default, textures will
 * use the global texture filtering settings. Using this function will override
 * those settings for this particular texture.
 *
 * @param mode		Texture filtering mode to use.
 */
void TextureBase::setFilterMode(SamplerFilterMode mode) {
	// TODO: global filtering defaults.
	if(mode != m_filterMode) {
		m_filterMode = mode;
		updateSamplerState();
	}
}

/**
 * Set the anisotropy level.
 *
 * When the filtering mode is set to anisotropic, this sets the degree of
 * anisotropy used by the filtering process. Unless the filtering mode has been
 * overridden from the global defaults using setFilterMode(), this parameter is
 * ignored.
 *
 * @param anisotropy	Degree of anisotropy for anisotropic filtering.
 */
void TextureBase::setAnisotropy(unsigned anisotropy) {
	if(anisotropy != m_anisotropy) {
		m_anisotropy = anisotropy;
		updateSamplerState();
	}
}

/**
 * Set the texture addressing mode.
 *
 * This sets the method used for resolving texture coordinates outside the
 * (0, 1) range. When it is set to clamp, texture coordinates are clamped to
 * the (0, 1) range. When it is set to wrap, texture coordinates are wrapped
 * around, causing the texture to repeat.
 *
 * @param mode		Texture addressing mode to use.
 */
void TextureBase::setAddressMode(SamplerAddressMode mode) {
	if(mode != m_addressMode) {
		m_addressMode = mode;
		updateSamplerState();
	}
}

/** Recreate the texture sampler state. */
void TextureBase::updateSamplerState() {
	GPUSamplerStateDesc desc;
	desc.filterMode = m_filterMode;
	desc.maxAnisotropy = m_anisotropy;
	desc.addressU = desc.addressV = desc.addressW = m_addressMode;
	m_sampler = g_gpu->createSamplerState(desc);
}

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
	IntRect area(0, 0, m_gpu->width(), m_gpu->height());
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
void Texture2D::update(const IntRect &area, const void *data, bool updateMipmap) {
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
void Texture2D::update(unsigned mip, const IntRect &area, const void *data) {
	check(mip < mips());

	m_gpu->update(area, data, mip);
}
