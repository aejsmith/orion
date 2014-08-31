/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine asset manager.
 *
 * I think most of the way this works now will be temporary. In future I think
 * it will be changed so that assets will be stored as serialized objects, which
 * are then unserialized into the assets. This will include asset data as well
 * as attributes.
 *
 * Loaders would become importers that initially create an asset from a file in
 * the editor, but they would not be used at runtime. The external interface
 * used by the rest of the engine would probably remain the same.
 *
 * The way it works now is nice to begin with though as it means I can just dump
 * files in whatever format into the assets directory and use them, and easily
 * fiddle with things with JSON attributes.
 */

#include "filesystem_asset_store.h"

#include "asset/asset_factory.h"
#include "asset/asset_loader.h"
#include "asset/asset_manager.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <memory>

/** Initialize the asset manager.
 * @param config	Engine configuration structure. */
AssetManager::AssetManager(const EngineConfiguration &config) {
	/* Create and register the default store factories. */
	register_store_factory(new FilesystemAssetStoreFactory);

	/* Mount the engine internal assets. */
	if(!mount_store("engine", "fs", "engine/assets"))
		orion_abort("Failed to mount engine asset store");

	/* Mount all asset stores specified in the configuration. */
	for(const EngineConfiguration::AssetStoreTuple &store : config.asset_stores) {
		std::string name, type, path;
		std::tie(name, type, path) = store;

		if(!mount_store(name, type, path)) {
			orion_abort(
				"Failed to mount asset store '%s' (%s://%s)",
				name.c_str(), type.c_str(), path.c_str());
		}
	}
}

/** Destroy the asset manager. */
AssetManager::~AssetManager() {
	// TODO: Destroy assets and still registered factories/loaders.
}

/**
 * Look up an asset.
 *
 * Looks up an asset by path. The asset will not necessarily be loaded, if this
 * is required it should be explicitly loaded by calling Asset::load() on the
 * returned asset, or call load() instead which requests that the asset is
 * loaded.
 *
 * @param path		Path to the asset.
 *
 * @return		Pointer to asset, or null if asset not found.
 */
AssetPtr AssetManager::lookup(const std::string &path) {
	/* Look up the path in the cache of known assets. */
	auto exist = m_assets.find(path);
	if(exist != m_assets.end())
		return AssetPtr(exist->second);

	/* No asset found, we must create a new one. */
	AssetLoadState state;
	if(!open(path, state))
		return nullptr;

	/* Create the asset. */
	Asset *asset = state.factory->create(this, path);
	if(!asset)
		return nullptr;

	m_assets.insert(std::make_pair(path, asset));
	return AssetPtr(asset);
}

/**
 * Look up an asset and load it.
 *
 * Looks up an asset by path, and requests that it is loaded. This is equivalent
 * to calling lookup() and then Asset::load() on the returned asset.
 *
 * @param path		Path to the asset.
 *
 * @return		Pointer to asset, or null if asset not found.
 */
AssetPtr AssetManager::load(const std::string &path) {
	AssetPtr asset = lookup(path);

	if(asset)
		asset->load();

	return asset;
}

/**
 * Open an asset.
 *
 * Opens an asset. This function will find an asset given a path, then open its
 * metadata and data streams, parse the metadata, and find the factory and
 * loader for the asset. All of this is returned in a state structure to be used
 * to instantiate a new asset, or load in an asset's data.
 *
 * @param path		Path to the asset.
 * @param state		State structure to fill in.
 *
 * @return		Whether the asset was opened successfully.
 */
bool AssetManager::open(const std::string &path, AssetLoadState &state) {
	/* Look up the store, and the path within the store. */
	std::string store_path;
	AssetStore *store = find_store(path, store_path);
	if(!store) {
		orion_log(LogLevel::kError, "Could not find asset '%s' (invalid store)", path.c_str());
		return false;
	}

	/* Open the asset from the store. */
	if(!store->open(store_path, state)) {
		orion_log(LogLevel::kError, "Could not find asset '%s' (store error)", path.c_str());
		return false;
	}

	orion_assert(state.metadata || state.data);

	/* Create a metadata JSON stream. If we don't have a metadata stream
	 * from the asset store, we'll create an empty one and infer the type
	 * from the loader. */
	rapidjson::Document metadata;
	if(state.metadata && state.metadata->size()) {
		std::unique_ptr<char[]> buf(new char[state.metadata->size() + 1]);
		buf[state.metadata->size()] = 0;
		if(!state.metadata->read(buf.get(), state.metadata->size())) {
			orion_log(LogLevel::kError, "Failed to read asset '%s' metadata", path.c_str());
			return false;
		}

		metadata.Parse(buf.get());
		if(metadata.HasParseError()) {
			const char *msg = rapidjson::GetParseError_En(metadata.GetParseError());
			orion_log(LogLevel::kError,
				"Parse error in '%s' metadata (at %zu): %s",
				path.c_str(), metadata.GetErrorOffset(), msg);
			return false;
		}
	}

	/* Look for a loader for the data stream, if any. */
	if(state.data) {
		state.loader = find_loader(state.type);
		if(!state.loader) {
			orion_log(LogLevel::kError,
				"Cannot load asset '%s' with unknown file type '%s'",
				path.c_str(), state.type.c_str());
			return false;
		}
	}

	/* Try to determine the asset type. If the metadata file defines a type,
	 * use that. Otherwise, infer it from the loader type. */
	if(metadata.HasMember("type")) {
		if(!metadata["type"].IsString()) {
			orion_log(LogLevel::kError, "Asset '%s' metadata is invalid", path.c_str());
			return false;
		}

		state.factory = find_factory(metadata["type"].GetString());
		if(!state.factory) {
			orion_log(LogLevel::kError,
				"Asset '%s' metadata specifies unknown type '%s'",
				path.c_str(), metadata["type"].GetString());
			return false;
		}

		/* Error if the loader's type does not match. */
		if(state.loader && strcmp(state.factory->type(), state.loader->asset_type()) != 0) {
			orion_log(LogLevel::kError,
				"Asset '%s' metadata specifies type '%s' not matching file type '%s'",
				path.c_str(), state.factory->type(), state.loader->asset_type());
			return false;
		}
	} else {
		/* Need a loader to infer the type. */
		if(!state.loader) {
			orion_log(LogLevel::kError,
				"Asset '%s' metadata does not specify type and no data to infer from",
				path.c_str());
			return false;
		}

		state.factory = find_factory(state.loader->asset_type());
		orion_assert(state.factory);
	}

	/* Get attributes from the metadata. */
	if(metadata.HasMember("attributes")) {
		if(!metadata["attributes"].IsObject()) {
			orion_log(LogLevel::kError, "Asset '%s' metadata is invalid", path.c_str());
			return false;
		}

		state.attributes = metadata["attributes"];
	} else {
		/* Create an empty object. */
		state.attributes.SetObject();
	}

	return true;
}

/**
 * Mount an asset store.
 *
 * Mounts an asset store into the asset tree. Asset stores are mounted at the
 * top level of the tree, so for example mounting an asset store with the name
 * 'game' makes its contents available under 'game/...'.
 *
 * @param name		Name to mount the store as.
 * @param type		Type of the store.
 * @param path		Path to the store. The interpretation of this path is
 *			entirely up to the asset store type. For a filesystem
 *			store this would be the filesystem path to where the
 *			store can be found.
 *
 * @return		Whether the store was mounted successfully.
 */
bool AssetManager::mount_store(const std::string &name, const std::string &type, const std::string &path) {
	/* Check for a name conflict. */
	if(m_stores.find(name) != m_stores.end()) {
		orion_log(LogLevel::kError, "Asset store '%s' already mounted", name.c_str());
		return false;
	}

	/* Look up the factory. */
	AssetStoreFactory *factory = find_store_factory(type);
	if(!factory) {
		orion_log(LogLevel::kError, "Unknown asset store type '%s'", type.c_str());
		return false;
	}

	/* Create the store. */
	AssetStore *store = factory->create(path);
	if(!store)
		return false;

	m_stores.insert(std::make_pair(name, store));
	orion_log(LogLevel::kInfo, "Mounted asset store '%s' (%s://%s)", name.c_str(), type.c_str(), path.c_str());
	return true;
}

/** Unmount an asset store.
 * @param name		Name of the store to unmount. */
void AssetManager::unmount_store(const std::string &name) {
	orion_abort("TODO: unmount store");
}

/** Find the store containing an asset.
 * @param path		Path to the asset.
 * @param store_path	String which will be set to the path to the asset within
 *			the store.
 * @return		Pointer to store found, null if store does not exist. */
AssetStore *AssetManager::find_store(const std::string &path, std::string &store_path) const {
	size_t pos = path.find('/');
	if(pos == std::string::npos)
		return nullptr;

	std::string store_name = path.substr(0, pos);
	store_path = path.substr(pos + 1);

	if(!store_name.size() || !store_path.size())
		return nullptr;

	auto store = m_stores.find(store_name);
	return (store != m_stores.end()) ? store->second : nullptr;
}

/**
 * Register an asset factory.
 *
 * Registers an asset factory. The factory becomes owned by the manager, if
 * the engine is shutdown while the factory is still registered, it will be
 * deleted.
 *
 * @param factory	Factory to register.
 */
void AssetManager::register_factory(AssetFactory *factory) {
	auto ret = m_factories.insert(std::make_pair(factory->type(), factory));
	orion_check(ret.second, "Registering asset factory '%s' that already exists", factory->type());
}

/**
 * Unregister an asset factory.
 *
 * Unregisters an asset factory. The factory will not be deleted, this must be
 * done manually.
 *
 * @param factory	Factory to unregister.
 */
void AssetManager::unregister_factory(AssetFactory *factory) {
	size_t ret = m_factories.erase(factory->type());
	orion_check(ret, "Unregistering asset factory '%s' that does not exist", factory->type());
}

/** Look up an asset factory by type.
 * @param type		Asset type name. */
AssetFactory *AssetManager::find_factory(const std::string &type) const {
	auto ret = m_factories.find(type);
	return (ret != m_factories.end()) ? ret->second : nullptr;
}

/**
 * Register an asset store factory.
 *
 * Registers an asset store factory. The factory becomes owned by the manager,
 * if the engine is shutdown while the factory is still registered, it will be
 * deleted.
 *
 * @param factory	Factory to register.
 */
void AssetManager::register_store_factory(AssetStoreFactory *factory) {
	auto ret = m_store_factories.insert(std::make_pair(factory->type(), factory));
	orion_check(ret.second, "Registering asset store factory '%s' that already exists", factory->type());
}

/**
 * Unregister an asset store factory.
 *
 * Unregisters an asset store factory. The factory will not be deleted, this
 * must be done manually.
 *
 * @param factory	Factory to unregister.
 */
void AssetManager::unregister_store_factory(AssetStoreFactory *factory) {
	// FIXME: Don't allow removing factories with active stores.
	size_t ret = m_store_factories.erase(factory->type());
	orion_check(ret, "Unregistering asset store factory '%s' that does not exist", factory->type());
}

/** Look up an asset store factory by type.
 * @param type		Asset store type name. */
AssetStoreFactory *AssetManager::find_store_factory(const std::string &type) const {
	auto ret = m_store_factories.find(type);
	return (ret != m_store_factories.end()) ? ret->second : nullptr;
}

/**
 * Register an asset loader.
 *
 * Registers an asset loader. The loader becomes owned by the manager, if the
 * 0engine is shutdown while the loader is still registered, it will be deleted.
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
	// FIXME: Don't allow removing loaders with active assets.
	size_t ret = m_loaders.erase(loader->type());
	orion_check(ret, "Unregistering asset loader '%s' that does not exist", loader->type());
}

/** Look up an asset loader by type.
 * @param type		File type string. */
AssetLoader *AssetManager::find_loader(const std::string &type) const {
	auto ret = m_loaders.find(type);
	return (ret != m_loaders.end()) ? ret->second : nullptr;
}
