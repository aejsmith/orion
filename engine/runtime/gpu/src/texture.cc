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
 * @brief               GPU texture class.
 */

#include "gpu/texture.h"

/** Initialize the texture.
 * @param desc          Descriptor containing texture parameters. */
GPUTexture::GPUTexture(const GPUTextureDesc &desc) :
    m_type      (desc.type),
    m_width     (desc.width),
    m_height    (desc.height),
    m_depth     (desc.depth),
    m_format    (desc.format),
    m_flags     (desc.flags),
    m_baseMip   (0),
    m_baseLayer (0)
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

/** Initialize the texture as a texture view.
 * @param desc          Descriptor for the view. */
GPUTexture::GPUTexture(const GPUTextureViewDesc &desc) :
    m_type      (desc.type),
    m_width     (desc.source->m_width),
    m_height    (desc.source->m_height),
    m_depth     ((desc.type == kTexture3D) ? desc.source->m_depth : desc.layers),
    m_format    (desc.format),
    m_mips      (desc.mips),
    m_flags     (0),
    m_source    (desc.source),
    m_baseMip   (desc.baseMip),
    m_baseLayer (desc.baseLayer)
{
    #if ORION_BUILD_DEBUG
        /* Source must not be a view. */
        check(!desc.source->m_source);

        check(m_baseMip < m_source->m_mips);

        const uint32_t sourceLayers = (m_source->m_type == kTextureCube)
                                          ? CubeFace::kNumFaces
                                          : m_source->m_depth;
        check(m_baseLayer < sourceLayers);

        switch (desc.type) {
            case kTexture2D:
                check(m_source->m_type == kTexture2D ||
                      m_source->m_type == kTexture2DArray ||
                      m_source->m_type == kTextureCube);
                break;

            case kTexture2DArray:
                check(m_source->m_type == kTexture2DArray);
                break;

            case kTextureCube:
                check(m_source->m_type == kTextureCube);
                break;

            case kTexture3D:
                check(m_source->m_type == kTexture3D);
                break;

        }
    #endif

    /* Automatically fill in up to the end of the texture if mip/layer counts
     * are specified as 0. */
    if (m_mips == 0) {
        m_mips = m_source->m_mips - m_baseMip;
        check(m_mips != 0);
    }

    if (m_depth == 0) {
        m_depth = m_source->m_depth - m_baseLayer;
        check(m_depth != 0);
    }
}
