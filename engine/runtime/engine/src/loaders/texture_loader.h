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
 * @brief               Texture loader classes.
 */

#pragma once

#include "engine/asset_loader.h"
#include "engine/texture.h"

/** Texture loader base class. */
class TextureLoader : public AssetLoader {
public:
    // FIXME: objgen can't detect that a class has unimplemented pure virtuals.
    CLASS("constructable": false);

    /** Addressing mode for sampling the texture. */
    PROPERTY() SamplerAddressMode addressMode;

    /** Whether to use an sRGB format. */
    PROPERTY() bool sRGB;
protected:
    TextureLoader();

    void applyAttributes(TextureBase *texture);
    PixelFormat getFinalFormat(PixelFormat format) const;
};

/** 2D texture loader base class. */
class Texture2DLoader : public TextureLoader {
public:
    CLASS();

    AssetPtr load() override;
protected:
    /**
     * Load the texture data.
     *
     * Load the texture data from the source file. This function is expected
     * to set the m_width, m_height, m_format and m_data fields.
     *
     * @return              Whether the texture data was loaded sucessfully.
     */
    virtual bool loadData() = 0;
protected:
    uint32_t m_width;                   /**< Width of the texture. */
    uint32_t m_height;                  /**< Height of the texture. */
    PixelFormat m_format;               /**< Format of the texture. */

    /** Buffer containing texture data. */
    std::unique_ptr<uint8_t []> m_buffer;
};

/** Cube texture loader class. */
class TextureCubeLoader : public TextureLoader {
public:
    CLASS();

    /** @return             File extension which this loader handles. */
    const char *extension() const override {
        /* We source our data from the source textures. */
        return nullptr;
    }

    AssetPtr load() override;

    /** Source textures for each face. */
    PROPERTY() Texture2DPtr positiveXFace;
    PROPERTY() Texture2DPtr negativeXFace;
    PROPERTY() Texture2DPtr positiveYFace;
    PROPERTY() Texture2DPtr negativeYFace;
    PROPERTY() Texture2DPtr positiveZFace;
    PROPERTY() Texture2DPtr negativeZFace;
};
