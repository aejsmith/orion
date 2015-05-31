/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               GPU texture class.
 */

#include "gpu/texture.h"

/** Initialize the texture as a texture.
 * @param desc          Descriptor containing texture parameters. */
GPUTexture::GPUTexture(const GPUTextureDesc &desc) :
    m_type(desc.type),
    m_width(desc.width),
    m_height(desc.height),
    m_depth(desc.depth),
    m_format(desc.format),
    m_flags(desc.flags)
{
    check(m_width > 0);
    check(m_height > 0);

    if (m_type == kTexture2DArray || m_type == kTexture3D) {
        check(m_depth > 0);
    } else {
        m_depth = 1;
    }

    if (m_type == kTextureCube)
        check(m_width == m_height);

    /* Clamp number of mip levels to a valid range. */
    uint32_t width = m_width;
    uint32_t height = m_height;
    uint32_t depth = (m_type == kTexture3D) ? m_depth : 0;
    unsigned maxMips = 1;
    while (width > 1 || height > 1 || depth > 1) {
        if (width > 1)
            width >>= 1;
        if (height > 1)
            height >>= 1;
        if (depth > 1)
            depth >>= 1;

        maxMips++;
    }

    m_mips = (desc.mips) ? glm::clamp(desc.mips, 0u, maxMips) : maxMips;
}
