/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine asset manager.
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

#include "asset/asset_loader.h"
#include "asset/asset_manager.h"

#include "core/engine.h"

#include "lib/filesystem.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <memory>

#include "loaders/tga_loader.h"

/** Initialize the asset manager. */
AssetManager::AssetManager() {
	/* Register asset search paths. */
	m_search_paths.insert(std::make_pair("engine", "engine/assets"));
	m_search_paths.insert(std::make_pair("game", "game/assets"));

	/* Register asset loaders. */
	register_loader(new TGALoader);
}

/** Destroy the asset manager. */
AssetManager::~AssetManager() {
	// TODO: Destroy assets and still registered loaders.
}

/**
 * Load an asset.
 *
 * Loads an asset by path. Asset paths are not filesystem paths, the asset
 * manager maintains its own namespace which maps into locations within the
 * filesystem. Asset paths must be relative.
 *
 * @param path		Path to the asset to load.
 *
 * @return		Pointer to loaded asset, or null if asset not found.
 */
AssetPtr AssetManager::load(const Path &path) {
	/* Look up the path in the cache of known assets. */
	Asset *exist = lookup_asset(path);
	if(exist)
		return AssetPtr(exist);
	
	/* Turn the asset path into a filesystem path. */
	auto search_path = m_search_paths.find(path.subset(0, 1).str());
	if(search_path == m_search_paths.end()) {
		orion_log(LogLevel::kError, "Could not find asset '%s'", path.c_str());
		return nullptr;
	}

	Path fs_path = Path(search_path->second) / path.subset(1);
	Path directory_path = fs_path.directory_name();
	std::string asset_name = fs_path.base_file_name();

	/* Open the directory. */
	std::unique_ptr<Directory> directory(g_engine->filesystem()->open_directory(directory_path));
	if(!directory) {
		orion_log(LogLevel::kError, "Could not find asset '%s'", path.c_str());
		return nullptr;
	}

	/* Iterate over entries to try to find the asset data/metadata. */
	std::unique_ptr<DataStream> data;
	std::unique_ptr<DataStream> metadata;
	std::string type;
	Directory::Entry entry;
	while(directory->next(entry)) {
		if(entry.type != FileType::kFile)
			continue;

		if(entry.name.base_file_name() == asset_name) {
			std::string entry_ext = entry.name.extension();
			Path file_path = directory_path / entry.name;

			if(entry_ext == "metadata") {
				metadata.reset(g_engine->filesystem()->open_file(file_path));
				if(!metadata) {
					orion_log(LogLevel::kError, "Could not open '%s'", file_path.c_str());
					return nullptr;
				}
			} else if(entry_ext.length()) {
				if(data) {
					orion_log(LogLevel::kError,
						"Asset '%s' has multiple data streams",
						path.c_str());
					return nullptr;
				}

				data.reset(g_engine->filesystem()->open_file(file_path));
				if(!data) {
					orion_log(LogLevel::kError, "Could not open '%s'", file_path.c_str());
					return nullptr;
				}

				type = entry_ext;
			}
		}
	}

	/* Succeeded if we have at least a data stream. */
	if(!data) {
		orion_log(LogLevel::kError, "Could not find asset '%s'", path.c_str());
		return nullptr;
	}

	/* Look for a loader for the asset. */
	AssetLoader *loader = lookup_loader(type);
	if(!loader) {
		orion_log(LogLevel::kError,
			"Cannot load asset '%s' with unknown file type '%s'",
			path.c_str(), type.c_str());
		return nullptr;
	}

	/* Create a metadata JSON stream. If we don't have a metadata stream
	 * from the filesystem, we'll just leave it empty so the loader sees
	 * no attributes. */
	rapidjson::Document attributes;
	if(metadata && metadata->size()) {
		std::unique_ptr<char[]> buf(new char[metadata->size() + 1]);
		buf[metadata->size()] = 0;
		if(!metadata->read(buf.get(), metadata->size())) {
			orion_log(LogLevel::kError, "Failed to read asset '%s' metadata", path.c_str());
			return nullptr;
		}

		attributes.Parse(buf.get());
		if(attributes.HasParseError()) {
			const char *msg = rapidjson::GetParseError_En(attributes.GetParseError());
			orion_log(LogLevel::kError,
				"Parse error in '%s' metadata (at %zu): %s",
				path.c_str(), attributes.GetErrorOffset(), msg);
			return nullptr;
		}
	} else {
		attributes.SetObject();
	}

	/* Create the asset. The loader should log an error if it fails. */
	Asset *asset = loader->load(data.get(), attributes, path.c_str());
	if(!asset)
		return nullptr;

	/* Mark the asset as managed and cache it. */
	asset->m_path = path.str();
	m_assets.insert(std::make_pair(path.str(), asset));

	orion_log(LogLevel::kDebug, "Loaded asset '%s' with file type '%s'", path.c_str(), type.c_str());
	return AssetPtr(asset);
}

/** Look up an asset in the cache.
 * @param path		Path to the asset.
 * @return		Pointer to asset if found, null if not. */
Asset *AssetManager::lookup_asset(const Path &path) const {
	auto ret = m_assets.find(path.str());
	return (ret != m_assets.end()) ? ret->second : nullptr;
}

/** Unregister an asset that is about to be destroyed.
 * @param asset		Asset to unregister. */
void AssetManager::unregister_asset(Asset *asset) {
	size_t ret = m_assets.erase(asset->path());
	orion_check(ret, "Destroying asset '%s' which is not in the cache", asset->path().c_str());
}

/**
 * Register an asset loader.
 *
 * Registers an asset loader. The loader becomes owned by the manager, if the
 * engine is shutdown while the loader is still registered, it will be deleted.
 *
 * @param loader	Loader to register.
 */
void AssetManager::register_loader(AssetLoader *loader) {
	auto ret = m_loaders.insert(std::make_pair(loader->type(), loader));
	orion_check(ret.second, "Registering asset loader '%s' that already exists", loader->type());
}

/**
 * Unregister an asset loader.
 *
 * Unregisters an asset loader. The loader will not be deleted, this must be
 * done manually.
 *
 * @param loader	Loader to unregister.
 */
void AssetManager::unregister_loader(AssetLoader *loader) {
	size_t ret = m_loaders.erase(loader->type());
	orion_check(ret, "Unregistering asset loader '%s' that does not exist", loader->type());
}

/** Look up an asset loader by type.
 * @param type		File type string.
 * @return		Pointer to loader if found, null if not. */
AssetLoader *AssetManager::lookup_loader(const std::string &type) const {
	auto ret = m_loaders.find(type);
	return (ret != m_loaders.end()) ? ret->second : nullptr;
}
