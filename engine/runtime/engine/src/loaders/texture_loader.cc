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

#include "texture_loader.h"

#include "gpu/gpu_manager.h"

/**
 * Base texture loader.
 */

/** Apply texture attributes.
 * @param texture       Texture to apply to. */
void TextureLoader::applyAttributes(TextureBase *texture) {
    texture->setAddressMode(this->addressMode);
}

/**
 * Base 2D texture loader.
 */

/** Load a 2D texture asset.
 * @return              Pointer to loaded asset, null on failure. */
AssetPtr Texture2DLoader::load() {
    if (!loadData())
        return nullptr;

    /* Create the texture, with mipmaps. TODO: Some formats will include
     * mipmaps and therefore not need them creating here. */
    Texture2DPtr texture = new Texture2D(
        m_width,
        m_height,
        m_format,
        0,
        GPUTexture::kAutoMipmap | GPUTexture::kRenderTarget);
    texture->update(m_buffer.get());

    /* Apply attributes. */
    applyAttributes(texture);

    return texture;
}

/**
 * Cube texture loader.
 */

/** Load a cube texture asset.
 * @return              Pointer to loaded asset, null on failure. */
AssetPtr TextureCubeLoader::load() {
    Texture2DPtr *faces[CubeFace::kNumFaces] = {
        &this->positiveXFace,
        &this->negativeXFace,
        &this->positiveYFace,
        &this->negativeYFace,
        &this->positiveZFace,
        &this->negativeZFace,
    };

    uint32_t size = 0;

    /* Validate source textures. */
    for (unsigned i = 0; i < CubeFace::kNumFaces; i++) {
        Texture2DPtr &face = *faces[i];

        if (!face) {
            logError("%s: Source texture for face %u is missing", m_path, i);
            return nullptr;
        }

        /* Ensure dimensions are correct. */
        if (face->width() != face->height()) {
            logError("%s: Source texture '%s' is not square", m_path, face->path().c_str());
            return nullptr;
        } else if (size) {
            if (face->width() != size) {
                logError("%s: Source texture '%s' dimensions do not match", m_path, face->path().c_str());
                return nullptr;
            }
        } else {
            size = face->width();
        }
    }

    /* Create the cube texture. TODO: Better choice for format, perhaps specify
     * in attributes or determine from source. */
    TextureCubePtr texture = new TextureCube(
        size,
        PixelFormat::kR8G8B8A8,
        0,
        GPUTexture::kAutoMipmap | GPUTexture::kRenderTarget);

    /* Copy source texture data into the cube texture. */
    for (unsigned i = 0; i < CubeFace::kNumFaces; i++) {
        Texture2DPtr &face = *faces[i];

        GPUTextureImageRef source(face->gpu());
        GPUTextureImageRef dest(texture->gpu(), i);
        g_gpuManager->blit(source, dest, glm::ivec2(0, 0), glm::ivec2(0, 0), glm::ivec2(size, size));
    }

    texture->gpu()->generateMipmap();

    applyAttributes(texture);

    return texture;
}
