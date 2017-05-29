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

#pragma once

#include "core/data_stream.h"

#include "engine/asset_manager.h"

#include <rapidjson/document.h>

/** Class which loads asset data. */
class AssetLoader : public Object {
public:
    CLASS();

    virtual ~AssetLoader() {}

    AssetPtr load(DataStream *data, const char *path);

    /** @return             Whether the loader requires data. */
    virtual bool requireData() const { return true; }
protected:
    AssetLoader() {}

    /** Load the asset.
     * @return              Pointer to loaded asset, null on failure. */
    virtual AssetPtr load() = 0;
protected:
    DataStream *m_data;                 /**< Asset data stream (if any). */
    const char *m_path;                 /**< Asset path being loaded. */
};

/** Asset loader factory class. */
class AssetLoaderFactory {
public:
    explicit AssetLoaderFactory(const char *type);
    virtual ~AssetLoaderFactory();

    static AssetLoader *create(const std::string &type);
protected:
    /** Create an asset loader of this type.
     * @return              Created asset loader. */
    virtual AssetLoader *create() const = 0;
private:
    const char *m_type;                 /**< File type that this factory is for. */
};

/** Implement an asset loader type.
 * @param className     Loader class name.
 * @param type          File type string. */
#define IMPLEMENT_ASSET_LOADER(className, type) \
    class className##Factory : public AssetLoaderFactory { \
    public: \
        className##Factory() : AssetLoaderFactory(type) {} \
        AssetLoader *create() const override { return new className(); } \
    private: \
        static className##Factory m_instance; \
    }; \
    className##Factory className##Factory::m_instance
