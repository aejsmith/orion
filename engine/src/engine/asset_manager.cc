/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Engine asset manager.
 *
 * The way this works now is somewhat temporary. At the moment we always load
 * in data from disk. I think in future we will store assets as serialized
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

#include "engine/asset_loader.h"
#include "engine/asset_manager.h"

/** Global asset manager instance. */
EngineGlobal<AssetManager> g_assetManager;

/** Initialize the asset manager. */
AssetManager::AssetManager() {
    /* Register asset search paths. */
    m_searchPaths.insert(std::make_pair("engine", "engine/assets"));
    m_searchPaths.insert(std::make_pair("game", "game/assets"));
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
    std::unique_ptr<Directory> directory(g_filesystem->openDirectory(directoryPath));
    if (!directory) {
        logError("Could not find asset '%s'", path.c_str());
        return nullptr;
    }

    /* Iterate over entries to try to find the asset data/metadata. */
    std::unique_ptr<DataStream> data;
    std::unique_ptr<DataStream> metadata;
    std::string type;
    Directory::Entry entry;
    while (directory->next(entry)) {
        if (entry.type != FileType::kFile)
            continue;

        if (entry.name.baseFileName() == assetName) {
            std::string entryExt = entry.name.extension();
            Path filePath = directoryPath / entry.name;

            if (entryExt == "metadata") {
                metadata.reset(g_filesystem->openFile(filePath));
                if (!metadata) {
                    logError("Failed to open '%s'", filePath.c_str());
                    return nullptr;
                }
            } else if (entryExt.length()) {
                if (data) {
                    logError("Asset '%s' has multiple data streams", path.c_str());
                    return nullptr;
                }

                data.reset(g_filesystem->openFile(filePath));
                if (!data) {
                    logError("Failed to open '%s'", filePath.c_str());
                    return nullptr;
                }

                type = entryExt;
            }
        }
    }

    /* Succeeded if we have at least a data stream. */
    if (!data) {
        logError("Could not find asset '%s'", path.c_str());
        return nullptr;
    }

    /* Look for a loader for the asset. */
    std::unique_ptr<AssetLoader> loader(AssetLoaderFactory::create(type));
    if (!loader) {
        logError("%s: Unknown file type '%s'", path.c_str(), type.c_str());
        return nullptr;
    }

    /* Create the asset. The loader should log an error if it fails. */
    AssetPtr asset = loader->load(data.get(), metadata.get(), path.c_str());
    if (!asset)
        return nullptr;

    /* Mark the asset as managed and cache it. */
    asset->m_path = path.str();
    m_assets.insert(std::make_pair(path.str(), asset.get()));

    logDebug("Loaded asset '%s' with file type '%s'", path.c_str(), type.c_str());
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
}
