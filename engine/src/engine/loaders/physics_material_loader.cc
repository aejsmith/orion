/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Physics material asset loader.
 */

#include "engine/asset_loader.h"

#include "physics/physics_material.h"

/** Asset loader for physics materials. */
class PhysicsMaterialLoader : public AssetLoader {
public:
    bool dataIsMetadata() const override { return true; }
    AssetPtr load() override;
private:
    PhysicsMaterialPtr m_physicsMaterial;
};

IMPLEMENT_ASSET_LOADER(PhysicsMaterialLoader, "physics_material");

/** Load a physics material asset.
 * @return              Pointer to loaded asset, null on failure. */
AssetPtr PhysicsMaterialLoader::load() {
    m_physicsMaterial = new PhysicsMaterial();

    const rapidjson::Value *value;

    if (m_attributes.HasMember("restitution")) {
        value = &m_attributes["restitution"];

        if (!value->IsNumber()) {
            logError("%s: 'restitution' attribute should be a number", m_path);
            return nullptr;
        }

        m_physicsMaterial->setRestitution(value->GetDouble());
    }
    if (m_attributes.HasMember("friction")) {
        value = &m_attributes["friction"];

        if (!value->IsNumber()) {
            logError("%s: 'friction' attribute should be a number", m_path);
            return nullptr;
        }

        m_physicsMaterial->setFriction(value->GetDouble());
    }

    return m_physicsMaterial;
}
