/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Filesystem asset store.
 */

#ifndef ORION_ASSET_FILESYSTEM_ASSET_STORE_H
#define ORION_ASSET_FILESYSTEM_ASSET_STORE_H

#include "asset/asset_store.h"

/** Filesystem-based asset store. */
class FilesystemAssetStore : public AssetStore {
public:
	FilesystemAssetStore(const std::string &path);
	~FilesystemAssetStore();

	bool init();

	bool open(const std::string &path, AssetLoadState &state) override;
private:
	std::string m_path;		/**< Base path to the asset store. */
};

/** Filesystem asset store factory. */
class FilesystemAssetStoreFactory : public AssetStoreFactory {
public:
	FilesystemAssetStoreFactory();

	AssetStore *create(const std::string &path) override;
};

#endif /* ORION_ASSET_FILESYSTEM_ASSET_STORE_H */
