/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Material loader class.
 */

#pragma once

#include "engine/asset_loader.h"

/** Material loader class. */
class MaterialLoader : public AssetLoader {
public:
	MaterialLoader() {}

	/** @return		File type (extension) that this loader is for. */
	const char *type() const override { return "material"; }

	Asset *load(DataStream *stream, rapidjson::Value &attributes, const char *path) override;
};
