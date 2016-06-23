/*
 * Copyright (C) 2016 Alex Smith
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
 * @brief               Serialised asset loader.
 */

#include "core/json_serialiser.h"

#include "engine/asset_loader.h"

#include "physics/physics_material.h"

/** Asset loader for serialised objects. */
class ObjectLoader : public AssetLoader {
public:
    AssetPtr load() override;
};

IMPLEMENT_ASSET_LOADER(ObjectLoader, "object");

/** Load a serialised asset.
 * @return              Pointer to loaded asset, null on failure. */
AssetPtr ObjectLoader::load() {
    std::vector<uint8_t> data(m_data->size());
    if (!m_data->read(&data[0], m_data->size())) {
        logError("%s: Failed to read asset data", m_path);
        return nullptr;
    }

    JSONSerialiser serialiser;
    AssetPtr asset = serialiser.deserialise<Asset>(data);
    if (!asset) {
        logError("%s: Error during deserialisation", m_path);
        return nullptr;
    }

    return asset;
}
