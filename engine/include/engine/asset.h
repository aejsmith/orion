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
 * @brief               Base asset class.
 */

#pragma once

#include "engine/object.h"

/**
 * Base class of all assets.
 *
 * All game assets (textures, meshes, etc) derive from this class. Managed
 * assets are ones which are stored on disk. These can be unloaded when they
 * are not needed and can be reloaded at a later time. Unmanaged assets are ones
 * created at runtime, these do not have any data on disk, and are lost when
 * they are destroyed.
 */
class Asset : public Object {
public:
    CLASS();

    /** @return             Whether the asset is managed. */
    bool managed() const { return m_path.length(); }
    /** @return             Path to the asset (empty for unmanaged assets). */
    const std::string &path() const { return m_path; }
protected:
    /**
     * Initialize an unmanaged asset.
     *
     * Initializes the asset as an unmanaged asset. The asset manager will
     * turn the asset into a managed one if it is loading the asset.
     */
    Asset() {}

    ~Asset() {}

    void released() override;

    /** Display details of the asset in the debug explorer. */
    virtual void explore() {}
private:
    std::string m_path;             /**< Path to the asset (empty for unmanaged assets). */

    friend class AssetManager;
};

/** Smart pointer to a certain type of asset. */
template <typename T> using TypedAssetPtr = ObjectPtr<T>;

/** Type of a generic asset pointer. */
using AssetPtr = TypedAssetPtr<Asset>;
