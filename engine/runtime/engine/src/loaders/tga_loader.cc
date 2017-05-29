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
 * @brief               TGA texture loader.
 *
 * TODO:
 *  - Support compressed images and 16bpp images (need 16-bit packed pixel
 *    formats).
 */

#include "texture_loader.h"

/** TGA texture loader class. */
class TGALoader : public Texture2DLoader {
public:
    CLASS();

    /** @return             File extension which this loader handles. */
    const char *extension() const override { return "tga"; }

    bool loadData() override;
private:
    /** TGA image file header. */
    #pragma pack(push, 1)
    struct Header {
        uint8_t idLength;
        uint8_t colourMapType;
        uint8_t imageType;
        uint16_t colourMapOrigin;
        uint16_t colourMapLength;
        uint8_t colourMapDepth;
        uint16_t xOrigin;
        uint16_t yOrigin;
        uint16_t width;
        uint16_t height;
        uint8_t depth;
        uint8_t imageDescriptor;
    };
    #pragma pack(pop)
};

#include "tga_loader.obj.cc"

/** Load a TGA file.
 * @return              Whether the texture data was loaded sucessfully. */
bool TGALoader::loadData() {
    Header header;
    if (!m_data->read(reinterpret_cast<char *>(&header), sizeof(header), 0)) {
        logError("%s: Failed to read asset data", m_path);
        return false;
    }

    /* Only support uncompressed RGB images for now. */
    if (header.imageType != 2) {
        logError("%s: Unsupported image format (%u)", m_path, header.imageType);
        return false;
    }

    if (header.depth != 24 && header.depth != 32) {
        logError("%s: Unsupported depth (%u)", m_path, header.depth);
        return false;
    }

    /* Determine image properties. */
    m_width = header.width;
    m_height = header.height;
    m_format = PixelFormat::kB8G8R8A8;

    /* Read in the data, which is after the ID and colour map. */
    size_t size = m_width * m_height * (header.depth / 8);
    uint64_t offset = sizeof(header) +
        header.idLength +
        (header.colourMapLength * (header.colourMapDepth / 8));

    std::unique_ptr<uint8_t []> buffer(new uint8_t[size]);

    if (!m_data->read(buffer.get(), size, offset)) {
        logError("%s: Failed to read asset data", m_path);
        return false;
    }

    if (header.depth == 24) {
        /* Bleh, NVIDIA Vulkan doesn't support RGB/BGR formats so we have to
         * convert to an alpha format. */
        m_buffer.reset(new uint8_t[m_width * m_height * 4]);

        for (size_t i = 0; i < m_width * m_height; i++) {
            m_buffer[(i * 4) + 0] = buffer[(i * 3) + 0];
            m_buffer[(i * 4) + 1] = buffer[(i * 3) + 1];
            m_buffer[(i * 4) + 2] = buffer[(i * 3) + 2];
            m_buffer[(i * 4) + 3] = 255;
        }
    } else {
        m_buffer = std::move(buffer);
    }

    return true;
}
