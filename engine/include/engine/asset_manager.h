/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Engine asset manager.
 */

#pragma once

#include "core/path.h"

#include "engine/asset.h"

#include <map>
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

	AssetPtr load(const Path &path);

	/** Load an asset of a certain type.
	 * @tparam AssetType	Type of the asset to load.
	 * @param path		Path to the asset.
	 * @return		Pointer to asset, or null if asset could not be
	 *			loaded or is incorrect type. */
	template <typename AssetType>
	TypedAssetPtr<AssetType> load(const Path &path) {
		static_assert(
			std::is_base_of<Asset, AssetType>::value,
			"AssetType is not derived from Asset");

		AssetPtr asset = load(path);
		return asset.dynamicCast<AssetType>();
	}
private:
	Asset *lookupAsset(const Path &path) const;
	void unregisterAsset(Asset *asset);
private:
	/**
	 * Map of known assets.
	 *
	 * Map of known assets. Note we store a raw pointer here instead of
	 * an AssetPtr because we don't want to increase the reference count.
	 * Assets remove themselves from here when they are destroyed.
	 *
	 * @todo		Replace with a radix tree.
	 */
	std::map<std::string, Asset *> m_assets;

	/** Asset search paths. */
	std::map<std::string, std::string> m_searchPaths;

	friend class Asset;
};

extern EngineGlobal<AssetManager> g_assetManager;
