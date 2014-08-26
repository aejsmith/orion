/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Base asset class.
 */

#include "asset/asset.h"
#include "asset/asset_manager.h"
#include "asset/asset_loader.h"

#include "core/engine.h"

/**
 * Initialize an unmanaged asset.
 *
 * Initializes the asset as an unmanaged asset. The asset state will be set to
 * loaded, and cannot be changed.
 */
Asset::Asset() :
	m_state(kLoadedState),
	m_manager(nullptr)
{}

/**
 * Initialize a managed asset.
 *
 * Initialize the asset as a managed asset. The asset state will be set to
 * unloaded. This constructor must be used when an asset is created through an
 * asset factory.
 *
 * @param manager	Manager creating the asset.
 * @param path		Path to the asset.
 */
Asset::Asset(AssetManager *manager, const std::string &path) :
	m_state(kUnloadedState),
	m_manager(manager),
	m_path(path)
{}

/**
 * Ensure that the asset is loaded.
 *
 * If the asset is not currently loaded, this function will load the asset from
 * its store. Unmanaged assets are always loaded, therefore this function has
 * no effect on them.
 *
 * @todo		Error handling: if an asset fails to load, we should
 *			use some default asset in its place, e.g. an error
 *			model/texture/material.
 */
void Asset::load() {
	if(m_state != kUnloadedState)
		return;

	orion_assert(m_manager);

	m_state = kLoadingState;

	/* Open the asset data. */
	AssetLoadState state;
	if(!m_manager->open(m_path, state))
		orion_abort("Failed to load asset '%s' (open)", m_path.c_str());

	// FIXME: When loading, we should check that the factory and loader are
	// still for the same type of asset.

	/* Call the asset type's load function. */
	if(!load_impl(state))
		orion_abort("Failed to load asset '%s' (asset)", m_path.c_str());

	/* If we have data, load it in. */
	if(state.data) {
		orion_assert(state.loader);

		if(!state.loader->load(this, state))
			orion_abort("Failed to load asset '%s' (loader)", m_path.c_str());
	}

	m_state = kLoadedState;
}

/**
 * Unload the asset.
 *
 * If the asset is loaded, unloads it. This function must not be called on
 * unmanaged assets.
 */
void Asset::unload() {
	orion_check(!managed(), "Cannot unload unmanaged assets");

	unload_impl();
	m_state = kUnloadedState;
}

/** Called when the asset reference count reaches 0. */
void Asset::released() {
	// TODO: Free the asset?
}
