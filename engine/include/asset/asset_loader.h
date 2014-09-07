/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Asset loader class.
 */

#ifndef ORION_ASSET_ASSET_LOADER_H
#define ORION_ASSET_ASSET_LOADER_H

#include "asset/asset_manager.h"

/** Class which loads asset data. */
class AssetLoader {
public:
	virtual ~AssetLoader() {}

	/** Load the asset.
	 * @param stream	Stream containing asset data.
	 * @param attributes	Attributes specified in metadata.
	 * @param path		Path to asset (supplied so that useful error
	 *			messages can be logged).
	 * @return		Pointer to loaded asset, null on failure. */
	virtual Asset *load(DataStream *stream, rapidjson::Value &attributes, const char *path) = 0;

	/** @return		File type (extension) that this loader is for. */
	const char *type() const { return m_type; }
protected:
	/** Initialize the asset loader.
	 * @param type		File type (extension) that the loader is for. */
	explicit AssetLoader(const char *type) :
		m_type(type)
	{}
private:
	const char *m_type;		/**< File type string. */
};

#endif /* ORION_ASSET_ASSET_LOADER_H */