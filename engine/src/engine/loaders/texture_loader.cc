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

#include "texture_loader.h"

/** Load a 2D texture asset.
 * @return              Pointer to loaded asset, null on failure. */
AssetPtr Texture2DLoader::load() {
    if (!loadData())
        return nullptr;

    /* Create the texture, with mipmaps. TODO: Some formats will include
     * mipmaps and therefore not need them creating here. */
    Texture2DPtr texture(new Texture2D(m_width, m_height, m_format, GPUTexture::kAutoMipmap | GPUTexture::kRenderTarget));
    texture->update(m_buffer.get());

    /* Parse parameters. */
    if (m_attributes.HasMember("addressMode")) {
        if (!m_attributes["addressMode"].IsString()) {
            logError("%s: 'addressMode' attribute should be a string", m_path);
            return nullptr;
        }

        const char *modeString = m_attributes["addressMode"].GetString();
        SamplerAddressMode mode;

        if (strcmp(modeString, "Clamp") == 0) {
            mode = SamplerAddressMode::kClamp;
        } else if (strcmp(modeString, "Wrap") == 0) {
            mode = SamplerAddressMode::kWrap;
        } else {
            logError("%s: Invalid value '%s' for 'addressMode' attribute", m_path, modeString);
            return nullptr;
        }

        texture->setAddressMode(mode);
    }

    return texture;
}
