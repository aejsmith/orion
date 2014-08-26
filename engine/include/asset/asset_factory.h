/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Asset factory class.
 */

#ifndef ORION_ASSET_ASSET_FACTORY_H
#define ORION_ASSET_ASSET_FACTORY_H

#include "asset/asset_manager.h"

/** Interface to create assets of a certain type. */
class AssetFactory {
public:
	virtual ~AssetFactory() {}

	/** Create an unloaded asset of this type.
	 * @param manager	Manager creating the asset.
	 * @param path		Path to the asset.
	 * @return		Pointer to created asset, null on failure. */
	virtual Asset *create(AssetManager *manager, const std::string &path) = 0;
	 
	/** @return		Asset type string. */
	const char *type() const { return m_type; }
protected:
	/** Initialize the asset factory.
	 * @param type		Asset type string. */
	explicit AssetFactory(const char *type) : m_type(type) {}
private:
	const char *m_type;		/**< Asset type string. */
};

#endif /* ORION_ASSET_ASSET_FACTORY_H */
