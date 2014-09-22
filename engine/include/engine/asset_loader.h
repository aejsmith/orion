/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Asset loader class.
 */

#pragma once

#include "core/data_stream.h"

#include "engine/asset_manager.h"

#include <rapidjson/document.h>

/** Class which loads asset data. */
class AssetLoader {
public:
	virtual ~AssetLoader() {}

	AssetPtr load(DataStream *data, DataStream *metadata, const char *path);
protected:
	AssetLoader() {}

	/**
	 * Whether the asset data file should be treated as metadata.
	 *
	 * Some asset types (e.g. materials) only exist as metadata. Asset data
	 * files are matched to a loader based on their extension, while
	 * metadata files have a .metadata extension. This would mean that
	 * metadata-only assets would require a dummy data file that could be
	 * used to identify the type as well as the main metadata file. This
	 * property avoids that by treating the main data file as the metadata
	 * file, so only one file (the one with the type extension) is needed.
	 * The load() method will receive the parsed contents of that file as
	 * the attributes argument, and a null stream argument.
	 *
	 * @return		Whether data file should be treated as metadata.
	 */
	virtual bool dataIsMetadata() const { return false; }

	/** Load the asset.
	 * @return		Pointer to loaded asset, null on failure. */
	virtual AssetPtr load() = 0;
protected:
	DataStream *m_data;			/**< Asset data stream (if any). */
	rapidjson::Document m_attributes;	/**< Asset attributes. */
	const char *m_path;			/**< Asset path being loaded. */
};

/** Asset loader factory class. */
class AssetLoaderFactory {
public:
	explicit AssetLoaderFactory(const char *type);
	virtual ~AssetLoaderFactory();

	static AssetLoader *create(const std::string &type);
protected:
	/** Create an asset loader of this type.
	 * @return		Created asset loader. */
	virtual AssetLoader *create() const = 0;
private:
	const char *m_type;		/**< File type that this factory is for. */
};

/** Implement an asset loader type.
 * @param className	Loader class name.
 * @param type		File type string. */
#define IMPLEMENT_ASSET_LOADER(className, type) \
	class className##Factory : public AssetLoaderFactory { \
	public: \
		className##Factory() : AssetLoaderFactory(type) {} \
		AssetLoader *create() const override { return new className(); } \
	private: \
		static className##Factory m_instance; \
	}; \
	className##Factory className##Factory::m_instance
