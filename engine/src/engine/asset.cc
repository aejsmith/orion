/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Base asset class.
 */

#include "engine/asset.h"
#include "engine/asset_manager.h"

/** Called when the asset reference count reaches 0. */
void Asset::released() {
    if (managed())
        g_assetManager->unregisterAsset(this);

    delete this;
}
