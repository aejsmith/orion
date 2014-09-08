/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Base asset class.
 */

#pragma once

#include "core/refcounted.h"

#include <string>

/**
 * Base class of all assets.
 *
 * All game assets (textures, meshes, etc) derive from this class. Managed
 * assets are ones which are stored on disk. These can be unloaded when they
 * are not needed and can be reloaded at a later time. Unmanaged assets are ones
 * created at runtime, these do not have any data on disk, and are lost when
 * they are destroyed.
 */
class Asset : public Refcounted {
public:
	/** @return		Whether the asset is managed. */
	bool managed() const { return m_path.length(); }
	/** @return		Path to the asset (empty for unmanaged assets). */
	const std::string &path() const { return m_path; }
protected:
	/**
	 * Initialize an unmanaged asset.
	 *
	 * Initializes the asset as an unmanaged asset. The asset manager will
	 * turn the asset into a managed one if it is loading the asset.
	 */
	Asset() {}

	void released() override;
private:
	std::string m_path;		/**< Path to the asset (empty for unmanaged assets). */

	friend class AssetManager;
};

/** Smart pointer to a certain type of asset. */
template <typename T> using TypedAssetPtr = ReferencePtr<T>;

/** Type of a generic asset pointer. */
typedef TypedAssetPtr<Asset> AssetPtr;
