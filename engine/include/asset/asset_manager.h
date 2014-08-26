/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine asset manager.
 */

#ifndef ORION_ASSET_ASSET_MANAGER_H
#define ORION_ASSET_ASSET_MANAGER_H

#include "asset/asset.h"

#include "core/engine.h"

#include "lib/data_stream.h"

#include <rapidjson/document.h>

#include <map>
#include <string>
#include <type_traits>

class AssetFactory;
class AssetLoader;
class AssetStore;
class AssetStoreFactory;

/** State used during asset loading. */
struct AssetLoadState {
	DataStream *metadata;		/**< Asset metadata stream. */
	rapidjson::Value attributes;	/**< Attributes parsed from metadata. */
	DataStream *data;		/**< Asset data stream. */
	std::string type;		/**< Type of asset data. */
	AssetFactory *factory;		/**< Factory used to create the asset. */
	AssetLoader *loader;		/**< Loader for asset data. */
public:
	/** Initialize the state. */
	AssetLoadState() :
		metadata(nullptr),
		data(nullptr),
		factory(nullptr),
		loader(nullptr)
	{}

	/** Destroy the state, closing data streams. */
	~AssetLoadState() {
		if(this->metadata)
			delete this->metadata;
		if(this->data)
			delete this->data;
	}
};

/**
 * Engine asset manager.
 *
 * This class manages all assets known to the engine. It is the interface
 * through which the rest of the engine accesses and loads assets.
 */
class AssetManager : Noncopyable {
public:
	explicit AssetManager(const EngineConfiguration &config);
	~AssetManager();

	/**
	 * Main asset methods.
	 */

	AssetPtr lookup(const std::string &path);
	AssetPtr load(const std::string &path);

	/**
	 * Look up an asset of a certain type and load it.
	 *
	 * Looks up an asset of a certain type by path, and requests that it is
	 * loaded. This is equivalent to calling lookup() and then Asset::load()
	 * on the returned asset.
	 *
	 * @tparam AssetType	Type of the asset to load.
	 *
	 * @param path		Path to the asset.
	 *
	 * @return		Pointer to asset, or null if asset not found.
	 */
	template <typename AssetType>
	TypedAssetPtr<AssetType> load(const std::string &path) {
		static_assert(
			std::is_base_of<Asset, AssetType>::value,
			"AssetType is not derived from Asset");

		AssetPtr asset = load(path);
		return TypedAssetPtr<AssetType>(dynamic_cast<AssetType *>(asset.get()));
	}
	
	/**
	 * Other methods.
	 */

	bool mount_store(const std::string &name, const std::string &type, const std::string &path);
	void unmount_store(const std::string &name);

	void register_factory(AssetFactory *factory);
	void unregister_factory(AssetFactory *factory);
	void register_loader(AssetLoader *loader);
	void unregister_loader(AssetLoader *loader);
	void register_store_factory(AssetStoreFactory *factory);
	void unregister_store_factory(AssetStoreFactory *factory);
private:
	bool open(const std::string &path, AssetLoadState &state);

	AssetStore *find_store(const std::string &path, std::string &store_path) const;
	AssetFactory *find_factory(const std::string &type) const;
	AssetLoader *find_loader(const std::string &type) const;
	AssetStoreFactory *find_store_factory(const std::string &type) const;
private:
	/**
	 * Map of known assets.
	 *
	 * Map of known assets. This doesn't necessarily include all assets in
	 * all stores, only ones which we currently have a created Asset object
	 * for that has references. Note we store a raw pointer here instead of
	 * an AssetPtr because we don't want to increase the reference count.
	 *
	 * @todo		Replace with a radix tree.
	 */
	std::map<std::string, Asset *> m_assets;

	/** Mounted asset stores. */
	std::map<std::string, AssetStore *> m_stores;
	/** Registered asset factories. */
	std::map<std::string, AssetFactory *> m_factories;
	/** Registered store factories. */
	std::map<std::string, AssetStoreFactory *> m_store_factories;
	/** Register asset loaders. */
	std::map<std::string, AssetLoader *> m_loaders;

	/** Asset needs to be able to call open(). */
	friend class Asset;
};

#endif /* ORION_ASSET_ASSET_MANAGER_H */
