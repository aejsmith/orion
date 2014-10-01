/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Asset loader class.
 */

#include "engine/asset_loader.h"

#include <rapidjson/error/en.h>

/** Load the asset.
 * @param data		Asset data stream.
 * @param metadata	Asset metadata stream.
 * @param path		Asset path being loaded.
 * @return		Pointer to loaded asset, null on failure. */
AssetPtr AssetLoader::load(DataStream *data, DataStream *metadata, const char *path) {
	m_path = path;

	if(dataIsMetadata()) {
		metadata = data;
		m_data = nullptr;
	} else {
		m_data = data;
	}

	/* Parse the metadata JSON stream. If we don't have a metadata stream
	 * from the filesystem, we'll just leave it empty so the loader sees
	 * no attributes. */
	if(metadata && metadata->size()) {
		std::unique_ptr<char[]> buf(new char[metadata->size() + 1]);
		buf[metadata->size()] = 0;
		if(!metadata->read(buf.get(), metadata->size())) {
			logError("%s: Failed to read metadata", path);
			return nullptr;
		}

		m_attributes.Parse(buf.get());
		if(m_attributes.HasParseError()) {
			const char *msg = rapidjson::GetParseError_En(m_attributes.GetParseError());
			logError("%s: Parse error in metadata (at %zu): %s", path, m_attributes.GetErrorOffset(), msg);
			return nullptr;
		}
	} else {
		m_attributes.SetObject();
	}

	return load();
}

/** @return		Registered loader factory map. */
static std::map<std::string, AssetLoaderFactory *> &assetLoaderFactoryMap() {
	static std::map<std::string, AssetLoaderFactory *> map;
	return map;
}

/** Initialize the asset loader factory.
 * @param type		File type (extension) that the loader is for. */
AssetLoaderFactory::AssetLoaderFactory(const char *type) :
	m_type(type)
{
	/* Register the loader factory. */
	auto ret = assetLoaderFactoryMap().insert(std::make_pair(m_type, this));
	checkMsg(ret.second, "Registering asset loader '%s' that already exists", m_type);
}

/** Destroy the asset loader factory. */
AssetLoaderFactory::~AssetLoaderFactory() {
	/* Unregister the loader factory. */
	assetLoaderFactoryMap().erase(m_type);
}

/** Create an asset loader for a file type.
 * @param type		File type to create for.
 * @return		Created asset loader if type known, null if not. */
AssetLoader *AssetLoaderFactory::create(const std::string &type) {
	auto ret = assetLoaderFactoryMap().find(type);
	return (ret != assetLoaderFactoryMap().end())
		? ret->second->create()
		: nullptr;
}
