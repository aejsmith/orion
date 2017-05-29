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
 * @brief               Engine asset manager.
 *
 * The way this works now is somewhat temporary. At the moment we always load
 * in data from disk. I think in future we will store assets as serialised
 * objects, which would include the asset data as well as attributes.
 *
 * Loaders would become importers that initially create an asset from a file in
 * the editor, but they would not be used at runtime. The external interface
 * used by the rest of the engine would probably remain the same.
 *
 * The way it works now is nice to begin with though as it means I can just dump
 * files in whatever format into the assets directory and use them, and easily
 * fiddle with things with JSON attributes.
 */

#include "core/filesystem.h"
#include "core/platform.h"

#include "engine/asset_loader.h"
#include "engine/asset_manager.h"
#include "engine/debug_manager.h"
#include "engine/debug_window.h"
#include "engine/json_serialiser.h"

/** Special file extensions. */
static const char *const kObjectFileExtension = "object";
static const char *const kLoaderFileExtension = "loader";

/** Global asset manager instance. */
AssetManager *g_assetManager;

/** Asset explorer debug overlay window. */
class AssetExplorerWindow : public DebugWindow {
public:
    AssetExplorerWindow() : DebugWindow("Asset Explorer") {}
    void render() override {
        ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiSetCond_Once);
        ImGui::SetNextWindowPosCenter(ImGuiSetCond_Once);

        if (begin())
            g_assetManager->explore();

        ImGui::End();
    }
};

/** Initialize the asset manager. */
AssetManager::AssetManager() {
    /* Register asset search paths. */
    m_searchPaths.insert(std::make_pair("engine", "engine/assets"));
    std::string gamePath = String::format("apps/%s/assets", Platform::getProgramName().c_str());
    logDebug("Game asset path is '%s'", gamePath.c_str());
    m_searchPaths.insert(std::make_pair("game", gamePath));

    g_debugManager->registerWindow(std::make_unique<AssetExplorerWindow>());
}

/** Destroy the asset manager. */
AssetManager::~AssetManager() {
    // TODO: Destroy assets.
}

/**
 * Load an asset.
 *
 * Loads an asset by path. Asset paths are not filesystem paths, the asset
 * manager maintains its own namespace which maps into locations within the
 * filesystem. Asset paths must be relative.
 *
 * @param path          Path to the asset to load.
 *
 * @return              Pointer to loaded asset, or null if asset not found.
 */
AssetPtr AssetManager::load(const Path &path) {
    /* Look up the path in the cache of known assets. */
    Asset *exist = lookupAsset(path);
    if (exist)
        return exist;

    /* Turn the asset path into a filesystem path. */
    auto searchPath = m_searchPaths.find(path.subset(0, 1).str());
    if (searchPath == m_searchPaths.end()) {
        logError("Could not find asset '%s'", path.c_str());
        return nullptr;
    }

    Path fsPath = Path(searchPath->second) / path.subset(1);
    Path directoryPath = fsPath.directoryName();
    std::string assetName = fsPath.baseFileName();

    /* Open the directory. */
    std::unique_ptr<Directory> directory(Filesystem::openDirectory(directoryPath));
    if (!directory) {
        logError("Could not find asset '%s'", path.c_str());
        return nullptr;
    }

    /* Iterate over entries to try to find the asset data and a corresponding
     * loader. */
    std::unique_ptr<DataStream> data;
    std::unique_ptr<DataStream> loaderData;
    std::string type;
    Directory::Entry entry;
    while (directory->next(entry)) {
        if (entry.type != FileType::kFile)
            continue;

        if (entry.name.baseFileName() == assetName) {
            std::string entryExt = entry.name.extension();
            Path filePath = directoryPath / entry.name;

            if (entryExt == kLoaderFileExtension) {
                loaderData.reset(Filesystem::openFile(filePath));
                if (!loaderData) {
                    logError("Failed to open '%s'", filePath.c_str());
                    return nullptr;
                }
            } else if (entryExt.length()) {
                if (data) {
                    logError("Asset '%s' has multiple data streams", path.c_str());
                    return nullptr;
                }

                data.reset(Filesystem::openFile(filePath));
                if (!data) {
                    logError("Failed to open '%s'", filePath.c_str());
                    return nullptr;
                }

                type = entryExt;
            }
        }
    }

    /* Succeeded if we have either stream. */
    if (!data && !loaderData) {
        logError("Could not find asset '%s'", path.c_str());
        return nullptr;
    }

    AssetPtr asset;

    /* Helper function used on both paths below to mark the asset as managed
     * and cache it. */
    auto addAsset =
        [&] (Object *object) {
            Asset *asset = static_cast<Asset *>(object);
            asset->m_path = path.str();
            m_assets.insert(std::make_pair(path.str(), asset));
        };

    if (type == kObjectFileExtension) {
        type.clear();

        /* This is a serialised object. */
        if (loaderData) {
            logError("%s: Serialised object cannot have a loader", path.c_str());
            return nullptr;
        }

        assert(data);

        std::vector<uint8_t> serialisedData(data->size());
        if (!data->read(&serialisedData[0], data->size())) {
            logError("%s: Failed to read asset data", path.c_str());
            return nullptr;
        }

        JSONSerialiser serialiser;

        /* We make the asset managed prior to calling its deserialise() method.
         * This is done for 2 reasons. Firstly, it makes the path available to
         * the deserialise() method. Secondly, it means that any references
         * back to the asset by itself or child objects will correctly be
         * resolved to it, rather than causing a recursive attempt to load the
         * asset. */
        serialiser.postConstructFunction = addAsset;

        asset = serialiser.deserialise<Asset>(serialisedData);
        if (!asset) {
            logError("%s: Error during object deserialisation", path.c_str());
            return nullptr;
        }
    } else {
        /* Get a loader for the asset. Use a serialised one if it exists, else
         * get a default one based on the file type. */
        ObjectPtr<AssetLoader> loader;
        if (loaderData) {
            std::vector<uint8_t> serialisedData(loaderData->size());
            if (!loaderData->read(&serialisedData[0], loaderData->size())) {
                logError("%s: Failed to read loader data", path.c_str());
                return nullptr;
            }

            JSONSerialiser serialiser;
            loader = serialiser.deserialise<AssetLoader>(serialisedData);
            if (!loader) {
                logError("%s: Error during loader deserialisation", path.c_str());
                return nullptr;
            }

            if (loader->requireData() && !data) {
                logError("%s: Asset '%s' has loader but missing data", path.c_str());
                return nullptr;
            }
        } else {
            assert(data);

            loader = AssetLoaderFactory::create(type);
            if (!loader) {
                logError("%s: Unknown file type '%s'", path.c_str(), type.c_str());
                return nullptr;
            }
        }

        /* Create the asset. The loader should log an error if it fails. */
        asset = loader->load(data.get(), path.c_str());
        if (!asset)
            return nullptr;

        addAsset(asset);
    }

    if (!type.empty()) {
        logDebug("Loaded asset '%s' from source file type '%s'", path.c_str(), type.c_str());
    } else {
        logDebug("Loaded asset '%s'", path.c_str());
    }

    return asset;
}

/** Look up an asset in the cache.
 * @param path          Path to the asset.
 * @return              Pointer to asset if found, null if not. */
Asset *AssetManager::lookupAsset(const Path &path) const {
    auto ret = m_assets.find(path.str());
    return (ret != m_assets.end()) ? ret->second : nullptr;
}

/** Unregister an asset that is about to be destroyed.
 * @param asset         Asset to unregister. */
void AssetManager::unregisterAsset(Asset *asset) {
    size_t ret = m_assets.erase(asset->path());
    checkMsg(ret, "Destroying asset '%s' which is not in the cache", asset->path().c_str());

    logDebug("Unregistered asset '%s'", asset->path().c_str());
}

/** Render the asset explorer window. */
void AssetManager::explore() {
    for (auto &entry : m_assets) {
        Asset *asset = entry.second;

        if (ImGui::TreeNode(asset, "%s", entry.first.c_str())) {
            const MetaClass &metaClass = asset->metaClass();
            ImGui::Text("Type: %s", metaClass.name());

            ImGui::Text("Refcount: %d", asset->refcount());

            asset->explore();

            ImGui::TreePop();
        }
    }
}
