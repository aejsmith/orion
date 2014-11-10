/**
 * @file
 * @copyright           2014 Alex Smith
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
    std::unique_ptr<char []> m_buffer;  /**< Buffer containing texture data. */
};
