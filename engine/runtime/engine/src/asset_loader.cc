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

/** Create an asset loader for a file type.
 * @param type          File type to create for.
 * @return              Created asset loader if type known, null if not. */
ObjectPtr<AssetLoader> AssetLoader::create(const std::string &type) {
    /* Map of file types to loader class. This is populated on first use of a
     * type to avoid having to search over the known classes for repeated loads
     * of a given type. */
    static std::map<std::string, const MetaClass *> typeMap;

    ObjectPtr<AssetLoader> loader;

    auto ret = typeMap.find(type);
    if (ret != typeMap.end()) {
        ObjectPtr<Object> object = ret->second->construct();
        loader = object.staticCast<AssetLoader>();
    } else {
        MetaClass::visit(
            [&] (const MetaClass &metaClass) {
                if (AssetLoader::staticMetaClass.isBaseOf(metaClass) && metaClass.isConstructable()) {
                    ObjectPtr<Object> object = metaClass.construct();
                    ObjectPtr<AssetLoader> tmpLoader = object.staticCast<AssetLoader>();

                    const char *extension = tmpLoader->extension();
                    if (extension != nullptr) {
                        typeMap.insert(std::make_pair(extension, &metaClass));

                        if (type == extension)
                            loader = tmpLoader;
                    }
                }
            });
    }

    return loader;
}
