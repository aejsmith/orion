/**
 * @file
 * @copyright           2014 Alex Smith
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
