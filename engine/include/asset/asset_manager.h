/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine asset manager.
 */

#ifndef ORION_ASSET_ASSET_MANAGER_H
#define ORION_ASSET_ASSET_MANAGER_H

#include "asset/asset.h"

#include "lib/path.h"

#include <rapidjson/document.h>

#include <map>
#include <string>
#include <type_traits>

class AssetLoader;

/**
 * Engine asset manager.
 *
 * This class manages all assets known to the engine. It is the interface
 * through which the rest of the engine accesses and loads assets.
 */
class AssetManager : Noncopyable {
public:
	AssetManager();
	~AssetManager();

	/**
	 * Main asset methods.
	 */

	AssetPtr load(const Path &path);

	/** Load an asset of a certain type.
	 * @tparam AssetType	Type of the asset to load.
	 * @param path		Path to the asset.
	 * @return		Pointer to asset, or null if asset could not be
	 *			loaded. */
	template <typename AssetType>
	TypedAssetPtr<AssetType> load(const Path &path) {
		static_assert(
			std::is_base_of<Asset, AssetType>::value,
			"AssetType is not derived from Asset");

		AssetPtr asset = load(path);
		return TypedAssetPtr<AssetType>(dynamic_cast<AssetType *>(asset.get()));
	}
	
	/**
	 * Other methods.
	 */

	void register_loader(AssetLoader *loader);
	void unregister_loader(AssetLoader *loader);
private:
	Asset *lookup_asset(const Path &path) const;
	void unregister_asset(Asset *asset);

	AssetLoader *lookup_loader(const std::string &type) const;
private:
	/**
	 * Map of known assets.
	 *
	 * Map of known assets. Note we store a raw pointer here instead of
	 * an AssetPtr because we don't want to increase the reference count.
	 *
	 * @todo		Replace with a radix tree.
	 */
	std::map<std::string, Asset *> m_assets;

	/** Registered asset loaders. */
	std::map<std::string, AssetLoader *> m_loaders;

	/** Asset search paths. */
	std::map<std::string, std::string> m_search_paths;

	friend class Asset;
};

#endif /* ORION_ASSET_ASSET_MANAGER_H */
