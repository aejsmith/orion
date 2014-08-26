/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Asset store class.
 */

#ifndef ORION_ASSET_ASSET_STORE_H
#define ORION_ASSET_ASSET_STORE_H

#include "asset/asset_manager.h"

/**
 * Source for assets.
 *
 * An asset store is a location in which game assets can be found, for example
 * a directory on the filesystem. It provides an interface to the asset manager
 * for searching available assets and reading in their data.
 */
class AssetStore : Noncopyable {
public:
	virtual ~AssetStore() {}

	/**
	 * Open an asset.
	 *
	 * Opens an asset from this store. This function should open at least
	 * one of the metadata and data streams for the asset and set them in
	 * the supplied state structure. If a data stream is opened, the type
	 * field should additionally be set in the state structure.
	 *
	 * @param path		Path to the asset, relative to the store root.
	 * @param state		Asset loading state structure.
	 *
	 * @return		Whether the asset was successfully opened.
	 */
	virtual bool open(const std::string &path, AssetLoadState &state) = 0;
protected:
	AssetStore() {}
};

/** Factory to create an asset store. */
class AssetStoreFactory {
public:
	/** Destroy the asset store factory. */
	virtual ~AssetStoreFactory() {}

	/** Create an asset store of this type.
	 * @param path		Asset store path (interpretation is entirely
	 *			up to this method). */
	virtual AssetStore *create(const std::string &path) = 0;

	/** @return		Name of the asset store type. */
	const char *type() const { return m_type; }
protected:
	/** Create the factory.
	 * @param type		Name of the asset store type. */
	explicit AssetStoreFactory(const char *type) : m_type(type) {}
private:
	const char *m_type;		/**< Name of the asset store type. */
};

#endif /* ORION_ASSET_ASSET_STORE_H */
