/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Base asset class.
 */

#ifndef ORION_ASSET_ASSET_H
#define ORION_ASSET_ASSET_H

#include "lib/refcounted.h"

#include <string>

struct AssetLoadState;
class AssetManager;

/**
 * Base class of all assets.
 *
 * All game assets (textures, meshes, etc) derive from this class. Managed
 * assets are ones which are stored on disk. These can be unloaded when they
 * are not needed and can be reloaded at a later time.
 *
 * Unmanaged assets can be created at runtime, these do not have any data on
 * disk, and cannot be unloaded.
 */
class Asset : public Refcounted {
public:
	/** State of an asset. */
	enum State {
		kUnloadedState,		/**< Asset data is not loaded. */
		kLoadingState,		/**< Asset is being loaded. */
		kLoadedState,		/**< Asset data is in memory. */
	};
public:
	void load();
	void unload();

	/** @return		Current asset state. */
	State state() const { return m_state; }
	/** @return		Whether the asset is managed. */
	bool managed() const { return m_manager; }
	/** @return		Path to the asset (only valid for managed assets). */
	const std::string &path() const { return m_path; }
protected:
	Asset();
	Asset(AssetManager *manager, const std::string &path);

	void released() override;

	/**
	 * Type-specific load method.
	 *
	 * This function is called when the asset is being loaded. It is called
	 * before the AssetLoader's load method is called. It should reset all
	 * asset state to some defaults, then apply any attributes specified
	 * in the metadata. Once this function returns success, the loader will
	 * be called to load the actual asset data, if any.
	 *
	 * @param state		Asset loading state.
	 *
	 * @return		Whether the asset was successfully loaded.
	 */
	virtual bool load_impl(AssetLoadState &state) = 0;

	/**
	 * Type-specific unload method.
	 *
	 * This function is called when the asset is being unloaded. It should
	 * free all resources and asset data.
	 */
	virtual void unload_impl() = 0;
private:
	State m_state;			/**< Current state of the asset. */
	AssetManager *m_manager;	/**< Manager for the asset. */
	std::string m_path;		/**< Path to the asset (only valid for managed assets). */
};

/** Smart pointer to a certain type of asset. */
template <typename T> using TypedAssetPtr = ReferencePtr<T>;

/** Type of a generic asset pointer. */
typedef TypedAssetPtr<Asset> AssetPtr;

#endif /* ORION_ASSET_ASSET_H */
