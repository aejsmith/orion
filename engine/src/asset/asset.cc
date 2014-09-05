/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Base asset class.
 */

#include "asset/asset.h"
#include "asset/asset_manager.h"

#include "core/engine.h"

/** Called when the asset reference count reaches 0. */
void Asset::released() {
	if(managed())
		g_engine->assets()->unregister_asset(this);

	delete this;
}
