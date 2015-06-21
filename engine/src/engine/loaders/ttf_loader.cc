/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               TTF font loader.
 */

#include "engine/asset_loader.h"
#include "engine/font.h"

/** TTF font asset loader. */
class TTFLoader : public AssetLoader {
public:
    AssetPtr load() override;
};

IMPLEMENT_ASSET_LOADER(TTFLoader, "ttf");

/** Load a TTF font asset.
 * @return              Pointer to loaded asset, null on failure. */
AssetPtr TTFLoader::load() {
    std::unique_ptr<char[]> data(new char[m_data->size()]);
    if (!m_data->read(data.get(), m_data->size())) {
        logError("%s: Failed to read asset data", m_path);
        return nullptr;
    }

    FontPtr font(new Font);
    if (!font->setData(std::move(data), m_data->size()))
        return nullptr;

    return font;
}
