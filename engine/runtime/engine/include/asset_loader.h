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

    /** @return             File extension which this loader handles, or null
     *                      if the loader does not require any additional
     *                      data. */
    virtual const char *extension() const = 0;

    AssetPtr load(DataStream *data, const char *path);

    /** @return             Whether the loader requires data. */
    bool requireData() const { return extension() != nullptr; }

    static ObjectPtr<AssetLoader> create(const std::string &type);
protected:
    AssetLoader() {}

    /** Load the asset.
     * @return              Pointer to loaded asset, null on failure. */
    virtual AssetPtr load() = 0;
protected:
    DataStream *m_data;                 /**< Asset data stream (if any). */
    const char *m_path;                 /**< Asset path being loaded. */
};
