/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GPU texture class.
 */

#include "gpu/texture.h"

/** Clamp the number of mip levels to a valid range.
 * @param mips		Supplied number of mip levels.
 * @param width		Width of the image in pixels.
 * @param height	Height of the image in pixels, or 0.
 * @param depth		Depth of the image in pixels, or 0.
 * @return		Maximum number of mipmap levels in the image. */
static unsigned clamp_mip_levels(unsigned mips, uint32_t width, uint32_t height, uint32_t depth) {
	unsigned max_mips;

	for(max_mips = 1; width > 1 || height > 1 || depth > 1; max_mips++) {
		if(width > 1)
			width >>= 1;
		if(height > 1)
			height >>= 1;
		if(depth > 1)
			depth >>= 1;
	}

	return (mips) ? glm::clamp(mips, 0u, max_mips) : max_mips;
}

/** Initialize the texture as a 2D texture.
 * @param desc		Descriptor containing texture parameters. */
GPUTexture::GPUTexture(const GPUTexture2DDesc &desc) :
	m_type(kTexture2D),
	m_width(desc.width),
	m_height(desc.height),
	m_depth(1),
	m_format(desc.format),
	m_mips(clamp_mip_levels(desc.mips, desc.width, desc.height, 0)),
	m_flags(desc.flags)
{
	orion_assert(m_width > 0);
	orion_assert(m_height > 0);
}

/** Initialize the texture as a 2D array texture.
 * @param desc		Descriptor containing texture parameters. */
GPUTexture::GPUTexture(const GPUTexture2DArrayDesc &desc) :
	m_type(kTexture2DArray),
	m_width(desc.width),
	m_height(desc.height),
	m_depth(desc.layers),
	m_format(desc.format),
	m_mips(clamp_mip_levels(desc.mips, desc.width, desc.height, 0)),
	m_flags(desc.flags)
{
	orion_assert(m_width > 0);
	orion_assert(m_height > 0);
	orion_assert(m_depth > 0);
}

/** Initialize the texture as a cube texture.
 * @param desc		Descriptor containing texture parameters. */
GPUTexture::GPUTexture(const GPUTextureCubeDesc &desc) :
	m_type(kTextureCube),
	m_width(desc.size),
	m_height(desc.size),
	m_depth(6),
	m_format(desc.format),
	m_mips(clamp_mip_levels(desc.mips, desc.size, desc.size, 0)),
	m_flags(desc.flags)
{
	orion_assert(m_width > 0);
}

/** Initialize the texture as a 3D texture.
 * @param desc		Descriptor containing texture parameters. */
GPUTexture::GPUTexture(const GPUTexture3DDesc &desc) :
	m_type(kTexture3D),
	m_width(desc.width),
	m_height(desc.height),
	m_depth(desc.depth),
	m_format(desc.format),
	m_mips(clamp_mip_levels(desc.mips, desc.width, desc.height, desc.depth)),
	m_flags(desc.flags)
{
	orion_assert(m_width > 0);
	orion_assert(m_height > 0);
	orion_assert(m_depth > 0);
}
