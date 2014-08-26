/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Asset loader class.
 */

#ifndef ORION_ASSET_ASSET_LOADER_H
#define ORION_ASSET_ASSET_LOADER_H

#include "asset/asset_manager.h"

/** Class which loads asset data from an asset store. */
class AssetLoader {
public:
	virtual ~AssetLoader() {}

	/** Load the asset.
	 * @param asset		Asset being loaded.
	 * @param state		Asset loading state.
	 * @return		Whether the object was loaded successfully. */
	virtual bool load(Asset *asset, AssetLoadState &state) = 0;

	/** @return		File type (extension) that this loader is for. */
	const char *type() const { return m_type; }
	/** @return		Type of the asset that this loader loads. */
	const char *asset_type() const { return m_asset_type; }
protected:
	/** Initialize the asset loader.
	 * @param type		File type (extension) that the loader is for.
	 * @param asset_type	Type of the asset that the loader loads. */
	AssetLoader(const char *type, const char *asset_type) :
		m_type(type),
		m_asset_type(asset_type)
	{}
private:
	const char *m_type;		/**< File type string. */
	const char *m_asset_type;	/**< Asset type string. */
};

#endif /* ORION_ASSET_ASSET_LOADER_H */
