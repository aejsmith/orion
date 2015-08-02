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
 * @brief               Texture asset class.
 */

#pragma once

#include "engine/asset.h"
#include "engine/render_target.h"

#include "gpu/state.h"
#include "gpu/texture.h"

/** Base texture asset class. */
class TextureBase : public Asset {
public:
    ~TextureBase() {}

    /** @return             Pixel format for the texture. */
    PixelFormat format() const { return m_gpu->format(); }
    /** @return             Number of mip levels. */
    unsigned mips() const { return m_gpu->mips(); }
    /** @return             Texture filtering mode. */
    SamplerFilterMode filterMode() const { return m_filterMode; }
    /** @return             Anisotropic filtering level. */
    unsigned anisotropy() const { return m_anisotropy; }
    /** @return             Addressing mode. */
    SamplerAddressMode addressMode() const { return m_addressMode; }

    void setFilterMode(SamplerFilterMode mode);
    void setAnisotropy(unsigned anisotropy);
    void setAddressMode(SamplerAddressMode mode);

    /** @return             GPU texture implementing this texture. */
    GPUTexture *gpu() const { return m_gpu; }
    /** @return             GPU sampler state for the texture. */
    GPUSamplerState *sampler() const { return m_sampler; }
protected:
    TextureBase();

    void updateSamplerState();
protected:
    GPUTexturePtr m_gpu;                /**< GPU texture pointer. */
    GPUSamplerStatePtr m_sampler;       /**< GPU sampler state. */

    /** Texture sampling parameters. */
    SamplerFilterMode m_filterMode;     /**< Filtering mode. */
    unsigned m_anisotropy;              /**< Anisotropic filtering level. */
    SamplerAddressMode m_addressMode;   /**< Addressing mode. */
};

/** Type of a base texture pointer. */
typedef TypedAssetPtr<TextureBase> TextureBasePtr;

/** Texture render target class. */
class RenderTexture : public RenderTarget {
public:
    uint32_t width() const override;
    uint32_t height() const override;
    void gpu(GPUTextureImageRef &ref) override;

    /** @return             Texture referred to by this render target. */
    TextureBase *texture() const { return m_texture; }
protected:
    RenderTexture(TextureBase *texture, unsigned layer);
private:
    TextureBase *m_texture;             /**< Texture that this target refers to. */
    unsigned m_layer;                   /**< Layer of texture. */

    friend class Texture2D;
};

/** Class implementing a 2D texture. */
class Texture2D : public TextureBase {
public:
    Texture2D(
        uint32_t width,
        uint32_t height,
        PixelFormat format = PixelFormat::kR8G8B8A8,
        unsigned mips = 0,
        uint32_t flags = GPUTexture::kAutoMipmap);
    ~Texture2D();

    void clear();

    void update(const void *data, bool updateMipmap = true);
    void update(const IntRect &area, const void *data, bool updateMipmap = true);
    void update(unsigned mip, const IntRect &area, const void *data);

    RenderTexture *renderTexture();

    /** @return             Width of the texture. */
    uint32_t width() const { return m_gpu->width(); }
    /** @return             Height of the texture. */
    uint32_t height() const { return m_gpu->height(); }
private:
    RenderTexture *m_renderTexture;     /**< Render target for the texture. */
};

/** Type of a 2D texture pointer. */
typedef TypedAssetPtr<Texture2D> Texture2DPtr;
