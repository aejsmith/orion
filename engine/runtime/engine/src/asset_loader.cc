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
 * @brief               Asset loader class.
 */

#include "engine/asset_loader.h"

/** Load the asset.
 * @param data          Asset data stream.
 * @param path          Asset path being loaded.
 * @return              Pointer to loaded asset, null on failure. */
AssetPtr AssetLoader::load(DataStream *data, const char *path) {
    m_data = data;
    m_path = path;
    return load();
}

/** @return             Registered loader factory map. */
static auto &assetLoaderFactoryMap() {
    static std::map<std::string, AssetLoaderFactory *> map;
    return map;
}

/** Initialize the asset loader factory.
 * @param type          File type (extension) that the loader is for. */
AssetLoaderFactory::AssetLoaderFactory(const char *type) :
    m_type(type)
{
    /* Register the loader factory. */
    auto ret = assetLoaderFactoryMap().insert(std::make_pair(m_type, this));
    checkMsg(ret.second, "Registering asset loader '%s' that already exists", m_type);
}

/** Destroy the asset loader factory. */
AssetLoaderFactory::~AssetLoaderFactory() {
    /* Unregister the loader factory. */
    assetLoaderFactoryMap().erase(m_type);
}

/** Create an asset loader for a file type.
 * @param type          File type to create for.
 * @return              Created asset loader if type known, null if not. */
AssetLoader *AssetLoaderFactory::create(const std::string &type) {
    auto ret = assetLoaderFactoryMap().find(type);
    return (ret != assetLoaderFactoryMap().end())
        ? ret->second->create()
        : nullptr;
}
