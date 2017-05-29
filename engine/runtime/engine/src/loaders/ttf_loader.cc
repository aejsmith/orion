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
 * @brief               TTF font loader.
 */

#include "engine/asset_loader.h"
#include "engine/font.h"

/** TTF font asset loader. */
class TTFLoader : public AssetLoader {
public:
    CLASS();

    /** @return             File extension which this loader handles. */
    const char *extension() const override { return "ttf"; }

    AssetPtr load() override;
};

#include "ttf_loader.obj.cc"

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
