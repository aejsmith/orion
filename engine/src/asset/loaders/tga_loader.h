/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		TGA texture loader.
 */

#ifndef ORION_ASSET_LOADERS_TGA_LOADER_H
#define ORION_ASSET_LOADERS_TGA_LOADER_H

#include "asset/asset_loader.h"

/** TGA texture loader class. */
class TGALoader : public AssetLoader {
public:
	TGALoader();

	Asset *load(DataStream *stream, rapidjson::Value &attributes, const char *path) override;
};

#endif /* ORION_ASSET_LOADERS_TGA_LOADER_H */
