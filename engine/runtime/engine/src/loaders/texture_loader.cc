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

#include "gpu/gpu_manager.h"

/** Parse common texture attributes.
 * @param path          Path to asset (for error output).
 * @param attributes    Attributes to parse.
 * @param texture       Texture to apply attributes to.
 * @return              Whether attributes were parsed successfully. */
static bool parseAttributes(
    const char *path,
    const rapidjson::Document &attributes,
    TextureBase *texture)
{
    if (attributes.HasMember("addressMode")) {
        if (!attributes["addressMode"].IsString()) {
            logError("%s: 'addressMode' attribute should be a string", path);
            return false;
        }

        const char *modeString = attributes["addressMode"].GetString();
        SamplerAddressMode mode;

        if (strcmp(modeString, "Clamp") == 0) {
            mode = SamplerAddressMode::kClamp;
        } else if (strcmp(modeString, "Wrap") == 0) {
            mode = SamplerAddressMode::kWrap;
        } else {
            logError("%s: Invalid value '%s' for 'addressMode' attribute", path, modeString);
            return false;
        }

        texture->setAddressMode(mode);
    }

    return true;
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

    /* Parse attributes. */
    if (!parseAttributes(m_path, m_attributes, texture))
        return nullptr;

    return texture;
}

/**
 * Cube texture loader.
 */

/** Cube texture loader class. */
class TextureCubeLoader : public AssetLoader {
public:
    bool dataIsMetadata() const override { return true; }
    AssetPtr load() override;
};

IMPLEMENT_ASSET_LOADER(TextureCubeLoader, "cube");

/** Names for each face attribute. */
static const char *faceAttributeNames[] = {
    "positiveXFace",
    "negativeXFace",
    "positiveYFace",
    "negativeYFace",
    "positiveZFace",
    "negativeZFace",
};

/** Load a cube texture asset.
 * @return              Pointer to loaded asset, null on failure. */
AssetPtr TextureCubeLoader::load() {
    uint32_t size = 0;
    Texture2DPtr faces[CubeFace::kNumFaces];

    /* Load the textures for each face. */
    for (unsigned i = 0; i < CubeFace::kNumFaces; i++) {
        const char *name = faceAttributeNames[i];

        if (!m_attributes.HasMember(name)) {
            logError("%s: '%s' attribute is missing", m_path, name);
            return nullptr;
        } else if (!m_attributes[name].IsString()) {
            logError("%s: '%s' attribute should be a string", m_path, name);
            return nullptr;
        }

        const char *sourcePath = m_attributes[name].GetString();
        faces[i] = g_assetManager->load<Texture2D>(sourcePath);

        /* Ensure dimensions are correct. */
        if (faces[i]->width() != faces[i]->height()) {
            logError("%s: Source texture '%s' is not square", m_path, sourcePath);
            return nullptr;
        } else if (size) {
            if (faces[i]->width() != size) {
                logError("%s: Source texture '%s' dimensions do not match", m_path, sourcePath);
                return nullptr;
            }
        } else {
            size = faces[i]->width();
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
        GPUTextureImageRef source(faces[i]->gpu());
        GPUTextureImageRef dest(texture->gpu(), i);
        g_gpuManager->blit(source, dest, glm::ivec2(0, 0), glm::ivec2(0, 0), glm::ivec2(size, size));
    }

    texture->gpu()->generateMipmap();

    /* Parse attributes. */
    if (!parseAttributes(m_path, m_attributes, texture))
        return nullptr;

    return texture;
}
