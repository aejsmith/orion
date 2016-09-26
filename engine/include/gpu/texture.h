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
 * @brief               GPU texture class.
 */

#pragma once

#include "core/hash.h"
#include "core/pixel_format.h"

#include "gpu/defs.h"

struct GPUTextureDesc;
struct GPUTextureImageRef;

/**
 * Class storing a texture on the GPU.
 *
 * This class stores texture data on the GPU. In most cases you should not use
 * this directly, rather you should use the texture asset classes. Since this
 * class has an API-specific implementation, instances must be created with
 * GPUManager::createTexture().
 */
class GPUTexture : public GPUObject {
public:
    /** Texture types. */
    enum Type {
        kTexture2D,                 /**< 2-dimensional texture. */
        kTexture2DArray,            /**< 2-dimensional texture array. */
        kTextureCube,               /**< Cube texture (6 2-dimensional faces). */
        kTexture3D,                 /**< 3-dimensional texture. */
    };

    /** Texture behaviour flags. */
    enum : uint32_t {
        /** Texture will have its mipmap automatically generated. */
        kAutoMipmap = (1 << 0),
        /** Texture will be used as a render target. */
        kRenderTarget = (1 << 1),
    };

    /** Update 2D texture area.
     * @param area          Area to update (2D rectangle).
     * @param data          Data to update with, in same format as texture.
     * @param layer         Array layer for 2D arrays, cube face for cube
     *                      textures, 0 otherwise.
     * @param mip           Mipmap level. */
    virtual void update(const IntRect &area, const void *data, unsigned mip = 0, unsigned layer = 0) = 0;

    /** Update 3D texture area.
     * @param area          Area to update (3D box).
     * @param data          Data to update with, in same format as texture.
     * @param mip           Mipmap level. */
    virtual void update(const IntBox &area, const void *data, unsigned mip = 0) = 0;

    /**
     * Generate mipmap images.
     *
     * Replaces image levels 1 through mips() with automatically generated
     * mipmap images based on level 0. The texture must have the kAutoMipmap
     * flag set.
     */
    virtual void generateMipmap() = 0;

    /** @return             Type of the texture. */
    Type type() const { return m_type; }
    /** @return             Width of the texture. */
    uint32_t width() const { return m_width; }
    /** @return             Height of the texture. */
    uint32_t height() const { return m_height; }
    /** @return             Depth of the texture (3D) or number of layers (array). */
    uint32_t depth() const { return m_depth; }
    /** @return             Pixel format for the texture. */
    PixelFormat format() const { return m_format; }
    /** @return             Number of mip levels. */
    unsigned mips() const { return m_mips; }
    /** @return             Texture behaviour flags. */
    uint32_t flags() const { return m_flags; }
    /** @return             Whether the texture is a texture view. */
    bool isView() const { return m_source; }
protected:
    explicit GPUTexture(const GPUTextureDesc &desc);
    explicit GPUTexture(const GPUTextureImageRef &image);
    ~GPUTexture() {}

    Type m_type;                    /**< Type of the texture. */
    uint32_t m_width;               /**< Width of the texture. */
    uint32_t m_height;              /**< Height of the texture. */
    uint32_t m_depth;               /**< Depth of the texture. */
    PixelFormat m_format;           /**< Pixel format. */
    unsigned m_mips;                /**< Number of mip levels. */
    uint32_t m_flags;               /**< Behaviour flags for the texture. */

    /** For texture views, the source texture. */
    GPUObjectPtr<GPUTexture> m_source;
};

/** Type of a pointer to a texture. */
using GPUTexturePtr = GPUObjectPtr<GPUTexture>;

/** Texture descriptor. */
struct GPUTextureDesc {
    GPUTexture::Type type;          /**< Type of the texture to create. */
    uint32_t width;                 /**< Width in pixels. */
    uint32_t height;                /**< Height in pixels (must be equal to width for kTextureCube). */
    uint32_t depth;                 /**< Depth in pixels (kTexture3D) or number of layers (kTexture2DArray). */
    PixelFormat format;             /**< Pixel format. */
    unsigned mips;                  /**< Number of mip levels (0 for full pyramid). */
    uint32_t flags;                 /**< Behaviour flags for the texture. */

    GPUTextureDesc() :
        mips(0),
        flags(0)
    {}

    SET_DESC_PARAMETER(setType, GPUTexture::Type, type);
    SET_DESC_PARAMETER(setWidth, uint32_t, width);
    SET_DESC_PARAMETER(setHeight, uint32_t, height);
    SET_DESC_PARAMETER(setDepth, uint32_t, depth);
    SET_DESC_PARAMETER(setFormat, PixelFormat, format);
    SET_DESC_PARAMETER(setMips, unsigned, mips);
    SET_DESC_PARAMETER(setFlags, uint32_t, flags);

    /** Compare this descriptor with another. */
    bool operator ==(const GPUTextureDesc &other) const {
        return
            type == other.type &&
            width == other.width &&
            height == other.height &&
            ((type != GPUTexture::kTexture2DArray && type != GPUTexture::kTexture3D) || depth == other.depth) &&
            format == other.format &&
            mips == other.mips &&
            flags == other.flags;
    }

    /** Get a hash from a texture descriptor. */
    friend size_t hashValue(const GPUTextureDesc &desc) {
        size_t hash = hashValue(desc.type);
        hash = hashCombine(hash, desc.width);
        hash = hashCombine(hash, desc.height);

        if (desc.type == GPUTexture::kTexture2DArray || desc.type == GPUTexture::kTexture3D)
            hash = hashCombine(hash, desc.depth);

        hash = hashCombine(hash, desc.format);
        hash = hashCombine(hash, desc.mips);
        hash = hashCombine(hash, desc.flags);
        return hash;
    }
};

/** Reference to a specific image (layer and mip) within a texture. */
struct GPUTextureImageRef {
    GPUTexture *texture;            /**< Texture to use. */
    unsigned layer;                 /**< Array layer/cube face. */
    unsigned mip;                   /**< Mip level. */
public:
    /** Initialize as a null reference. */
    GPUTextureImageRef() :
        texture(nullptr),
        layer(0),
        mip(0)
    {}

    /** Initialize to a texture. */
    GPUTextureImageRef(GPUTexture *inTexture, unsigned inLayer = 0, unsigned inMip = 0) :
        texture(inTexture),
        layer(inLayer),
        mip(inMip)
    {}

    /** Initialize to a texture. */
    GPUTextureImageRef(const GPUTexturePtr &inTexture, unsigned inLayer = 0, unsigned inMip = 0) :
        texture(inTexture),
        layer(inLayer),
        mip(inMip)
    {}

    /** @return             Whether this is a valid image reference. */
    explicit operator bool() const {
        return texture;
    }

    /** Compare this reference with another. */
    bool operator ==(const GPUTextureImageRef &other) const {
        return texture == other.texture && layer == other.layer && mip == other.mip;
    }

    /** Compare this reference with another. */
    bool operator !=(const GPUTextureImageRef &other) const {
        return !(*this == other);
    }

    /** Get a hash from a texture image reference. */
    friend size_t hashValue(const GPUTextureImageRef &ref) {
        size_t hash = hashValue(ref.texture);
        hash = hashCombine(hash, ref.layer);
        hash = hashCombine(hash, ref.mip);
        return hash;
    }
};
