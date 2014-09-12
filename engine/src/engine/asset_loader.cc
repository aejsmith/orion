/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Asset loader class.
 */

#include "engine/asset_loader.h"

/** Initialize the asset loader.
 * @param type		File type (extension) that this loader is for. */
AssetLoader::AssetLoader(const char *type) :
	m_type(type)
{
	/* Register the loader. */
	LoaderMap &loaders = loaderMap();
	auto ret = loaders.insert(std::make_pair(m_type, this));
	orionCheck(ret.second, "Registering asset loader '%s' that already exists", m_type);
}

/** Destroy the asset loader. */
AssetLoader::~AssetLoader() {
	/* Unregister the shader. */
	LoaderMap &loaders = loaderMap();
	loaders.erase(m_type);
}

/** @return		Registered loader map. */
AssetLoader::LoaderMap &AssetLoader::loaderMap() {
	static LoaderMap loaders;
	return loaders;
}

/** Look up an asset loader by type.
 * @param type		File type string.
 * @return		Pointer to loader if found, null if not. */
AssetLoader *AssetLoader::lookup(const std::string &type) {
	LoaderMap &loaders = loaderMap();
	auto ret = loaders.find(type);
	return (ret != loaders.end()) ? ret->second : nullptr;
}
