/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Base asset class.
 */

#include "engine/asset.h"
#include "engine/asset_manager.h"
#include "engine/engine.h"

/** Called when the asset reference count reaches 0. */
void Asset::released() {
	if(managed())
		g_engine->assets()->unregisterAsset(this);

	delete this;
}
