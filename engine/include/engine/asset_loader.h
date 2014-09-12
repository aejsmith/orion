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
	virtual ~AssetLoader();

	/* @return		File type (extension) that this loader is for. */
	const char *type() const { return m_type; }

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
	 * @param stream	Stream containing asset data.
	 * @param attributes	Attributes specified in metadata.
	 * @param path		Path to asset (supplied so that useful error
	 *			messages can be logged).
	 * @return		Pointer to loaded asset, null on failure. */
	virtual Asset *load(DataStream *stream, rapidjson::Value &attributes, const char *path) const = 0;

	static AssetLoader *lookup(const std::string &type);
protected:
	explicit AssetLoader(const char *type);
private:
	/** Type of the asset loader map. */
	typedef std::map<std::string, AssetLoader *> LoaderMap;
private:
	const char *m_type;		/**< File type that this loader is for. */

	static LoaderMap &loaderMap();
};
