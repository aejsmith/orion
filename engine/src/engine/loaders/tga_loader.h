/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		TGA texture loader.
 */

#pragma once

#include "engine/asset_loader.h"

/** TGA texture loader class. */
class TGALoader : public AssetLoader {
public:
	TGALoader() {}

	/** @return		File type (extension) that this loader is for. */
	const char *type() const override { return "tga"; }

	Asset *load(DataStream *stream, rapidjson::Value &attributes, const char *path) override;
};
