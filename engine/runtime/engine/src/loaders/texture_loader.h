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
 * @brief               Texture loader classes.
 */

#pragma once

#include "engine/asset_loader.h"
#include "engine/texture.h"

/** 2D texture loader base class. */
class Texture2DLoader : public AssetLoader {
public:
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
