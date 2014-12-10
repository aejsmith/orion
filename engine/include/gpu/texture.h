/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               GPU texture class.
 */

#pragma once

#include "core/hash.h"
#include "core/pixel_format.h"

#include "gpu/defs.h"

/** 2D texture descriptor. */
struct GPUTexture2DDesc {
    uint32_t width;                 /**< Width of the texture in pixels. */
    uint32_t height;                /**< Height of the texture in pixels. */
    PixelFormat format;             /**< Pixel format. */
    unsigned mips;                  /**< Number of mip levels (0 for full pyramid). */
    uint32_t flags;                 /**< Behaviour flags for the texture. */
};

/** 2D texture array descriptor. */
struct GPUTexture2DArrayDesc {
    uint32_t width;                 /**< Width of the texture in pixels. */
    uint32_t height;                /**< Height of the texture in pixels. */
    uint32_t layers;                /**< Number of array layers. */
    PixelFormat format;             /**< Pixel format. */
    unsigned mips;                  /**< Number of mip levels (0 for full pyramid). */
    uint32_t flags;                 /**< Behaviour flags for the texture. */
};

/** Cube texture descriptor. */
struct GPUTextureCubeDesc {
    uint32_t size;                  /**< Width/height of the texture in pixels. */
    PixelFormat format;             /**< Pixel format. */
    unsigned mips;                  /**< Number of mip levels (0 for full pyramid). */
    uint32_t flags;                 /**< Behaviour flags for the texture. */
};

/** 3D texture descriptor. */
struct GPUTexture3DDesc {
    uint32_t width;                 /**< Width of the texture in pixels. */
    uint32_t height;                /**< Height of the texture in pixels. */
    uint32_t depth;                 /**< Depth of the texture in pixels. */
    PixelFormat format;             /**< Pixel format. */
    unsigned mips;                  /**< Number of mip levels (0 for full pyramid). */
    uint32_t flags;                 /**< Behaviour flags for the texture. */
};

/**
 * Class storing a texture on the GPU.
 *
 * This class stores texture data on the GPU. In most cases you should not use
 * this directly, rather you should use the texture asset classes. Since this
 * class has an API-specific implementation, instances must be created with
 * GPUInterface::createTexture().
 */
class GPUTexture : public GPUResource {
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
public:
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
protected:
    explicit GPUTexture(const GPUTexture2DDesc &desc);
    explicit GPUTexture(const GPUTexture2DArrayDesc &desc);
    explicit GPUTexture(const GPUTextureCubeDesc &desc);
    explicit GPUTexture(const GPUTexture3DDesc &desc);
protected:
    Type m_type;                    /**< Type of the texture. */
    uint32_t m_width;               /**< Width of the texture. */
    uint32_t m_height;              /**< Height of the texture. */
    uint32_t m_depth;               /**< Depth of the texture. */
    PixelFormat m_format;           /**< Pixel format. */
    unsigned m_mips;                /**< Number of mip levels. */
    uint32_t m_flags;               /**< Behaviour flags for the texture. */
};

/** Type of a pointer to a texture. */
typedef GPUResourcePtr<GPUTexture> GPUTexturePtr;

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

/** Render target descriptor structure. */
struct GPURenderTargetDesc {
    /**
     * Array of colour render target descriptors. All array entries up to
     * numColours must be non-null.
     */
    GPUTextureImageRef colour[kMaxColourRenderTargets];

    /** Number of colour targets. */
    size_t numColours;

    /** Depth/stencil target. */
    GPUTextureImageRef depthStencil;
public:
    GPURenderTargetDesc() : numColours(0) {}

    /** Compare this reference with another. */
    bool operator ==(const GPURenderTargetDesc &other) const {
        if (numColours != other.numColours || depthStencil != other.depthStencil)
            return false;

        for (size_t i = 0; i < numColours; i++) {
            if (colour[i] != other.colour[i])
                return false;
        }

        return true;
    }

    /** Get a hash from a render target descriptor. */
    friend size_t hashValue(const GPURenderTargetDesc &desc) {
        size_t hash = hashValue(desc.numColours);
        hash = hashCombine(hash, desc.depthStencil);

        for (size_t i = 0; i < desc.numColours; i++)
            hash = hashCombine(hash, desc.colour[i]);

        return hash;
    }
};
