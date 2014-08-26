/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Filesystem asset store.
 *
 * The filesystem asset store uses a tree in the filesystem to find assets. An
 * asset has either 1 or 2 files: a JSON metadata file (asset_name.asset), a
 * data file (asset_name.<type>), or both. In the absence of a metadata file,
 * the asset type is inferred from its file extension. In the absence of a data
 * file, the asset type must be specified in the metadata file.
 *
 * @todo		Need to treat relative store paths as relative to the
 *			engine base directory. Use SDL_GetBasePath(), perhaps
 *			chdir() to it at startup?
 * @todo		Index directory contents. We really need a proper VFS-
 *			like asset store system so we can store data with each
 *			node in the tree.
 * @todo		Path manipulation class in FS library would help (both
 *			here and for generic asset store code).
 */

#include "filesystem_asset_store.h"

#include "core/engine.h"

#include "lib/fs.h"

/** Filesystem asset store constructor.
 * @param path		Base path for the asset store. */
FilesystemAssetStore::FilesystemAssetStore(const std::string &path) :
	m_path(path)
{}

/** Destroy the asset store. */
FilesystemAssetStore::~FilesystemAssetStore() {}

/** Properly initialize the asset store.
 * @return		Whether the asset store could be created. */
bool FilesystemAssetStore::init() {
	if(!m_path.length())
		return false;

	/* Check if the directory exists. */
	std::unique_ptr<Directory> root(fs::open_directory(m_path));
	if(!root) {
		orion_log(LogLevel::kError, "Could not open filesystem path '%s'", m_path.c_str());
		return false;
	}

	return true;
}

/** Open an asset.
 * @param path		Path to the asset, relative to the store root.
 * @param state		Asset loading state structure.
 * @return		Whether the asset was successfully opened. */
bool FilesystemAssetStore::open(const std::string &path, AssetLoadState &state) {
	/* Determine the directory path and asset name. */
	std::string absolute_path = m_path + '/' + path;
	std::string directory_path = absolute_path.substr(0, absolute_path.rfind('/'));
	std::string name = absolute_path.substr(absolute_path.rfind('/') + 1);

	/* Open the directory. */
	std::unique_ptr<Directory> directory(fs::open_directory(directory_path));
	if(!directory)
		return false;

	/* Iterate over entries to find entries with matching names. */
	Directory::Entry entry;
	while(directory->next(entry)) {
		if(entry.type != FileType::kFile)
			continue;

		std::string file_name = entry.name.substr(0, entry.name.rfind('.'));
		if(file_name == name) {
			std::string file_ext = entry.name.substr(entry.name.rfind('.') + 1);
			std::string file_path = directory_path + '/' + entry.name;

			if(file_ext == "asset") {
				state.metadata = fs::open_file(file_path);
			} else if(file_ext.length()) {
				if(state.data) {
					orion_log(LogLevel::kError,
						"Asset '%s' has multiple data streams",
						path.c_str());
					return false;
				}

				state.data = fs::open_file(file_path);
				state.type = file_ext;
			}
		}
	}

	/* Succeeded if we have at least one of the two streams. */
	return state.data || state.metadata;
}

/**
 * Asset store factory.
 */

/** Initialize the filesystem asset store factory. */
FilesystemAssetStoreFactory::FilesystemAssetStoreFactory() :
	AssetStoreFactory("fs")
{}

/** Create a filesystem asset store.
 * @param path		Asset store path. */
AssetStore *FilesystemAssetStoreFactory::create(const std::string &path) {
	FilesystemAssetStore *store = new FilesystemAssetStore(path);

	if(!store->init()) {
		delete store;
		return nullptr;
	}

	return store;
}
