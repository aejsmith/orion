/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Material asset loader.
 */

#include "engine/asset_loader.h"
#include "engine/material.h"

/** Material asset loader. */
class MaterialLoader : public AssetLoader {
public:
	bool dataIsMetadata() const override { return true; }
	AssetPtr load() override;
private:
	bool setParameter(const char *name, const rapidjson::Value &value);
private:
	MaterialPtr m_material;
};

IMPLEMENT_ASSET_LOADER(MaterialLoader, "omt");

/** Load a material.
 * @return		Pointer to loaded asset, null on failure. */
AssetPtr MaterialLoader::load() {
	if(!m_attributes.HasMember("shader") || !m_attributes["shader"].IsString()) {
		logError("%s: No/invalid shader specified", m_path);
		return nullptr;
	}

	ShaderPtr shader = g_assetManager->load<Shader>(m_attributes["shader"].GetString());
	m_material = new Material(shader);

	/* Set all parameters. FIXME: Should validate that all shader parameters
	 * have values set. */
	if(m_attributes.HasMember("parameters")) {
		if(!m_attributes["parameters"].IsObject()) {
			logError("%s: Invalid parameters specified, must be an object", m_path);
			return nullptr;
		}

		const rapidjson::Value &params = m_attributes["parameters"];
		for(auto it = params.MemberBegin(); it != params.MemberEnd(); ++it) {
			if(!setParameter(it->name.GetString(), it->value))
				return nullptr;
		}
	}

	return m_material;
}

/** Set a material parameter.
 * @param name		Parameter name.
 * @param value		Parameter value.
 * @return		Whether set successfully. */
bool MaterialLoader::setParameter(const char *name, const rapidjson::Value &value) {
	const ShaderParameter *shaderParam = m_material->shader()->lookupParameter(name);
	if(!shaderParam) {
		logError("%s: Unknown parameter '%s'", m_path, name);
		return false;
	}

	switch(shaderParam->type) {
	case ShaderParameter::kIntType:
		/* RapidJSON flags a value as being signed as long as
		 * it is within the range of a signed int. */
		if(!value.IsInt()) {
			logError("%s: Expected int for '%s'", m_path, name);
			return false;
		}

		m_material->setValue(name, value.GetInt());
		break;
	case ShaderParameter::kUnsignedIntType:
		/* Similar to the above. */
		if(!value.IsUint()) {
			logError("%s: Expected uint for '%s'", m_path, name);
			return false;
		}

		m_material->setValue(name, value.GetUint());
		break;
	case ShaderParameter::kFloatType:
		if(!value.IsNumber()) {
			logError("%s: Expected float for '%s'", m_path, name);
			return false;
		}

		m_material->setValue(name, static_cast<float>(value.GetDouble()));
		break;
	case ShaderParameter::kVec2Type:
		if(!value.IsArray() || value.Size() != 2
			|| !value[0u].IsNumber()
			|| !value[1u].IsNumber())
		{
			logError("%s: Expected vec2 for '%s'", m_path, name);
			return false;
		}

		m_material->setValue(name, glm::vec2(
			value[0u].GetDouble(),
			value[1u].GetDouble()));
		break;
	case ShaderParameter::kVec3Type:
		if(!value.IsArray() || value.Size() != 3
			|| !value[0u].IsNumber()
			|| !value[1u].IsNumber()
			|| !value[2u].IsNumber())
		{
			logError("%s: Expected vec3 for '%s'", m_path, name);
			return false;
		}

		m_material->setValue(name, glm::vec3(
			value[0u].GetDouble(),
			value[1u].GetDouble(),
			value[2u].GetDouble()));
		break;
	case ShaderParameter::kVec4Type:
		if(!value.IsArray() || value.Size() != 4
			|| !value[0u].IsNumber()
			|| !value[1u].IsNumber()
			|| !value[2u].IsNumber()
			|| !value[3u].IsNumber())
		{
			logError("%s: Expected vec4 for '%s'", m_path, name);
			return false;
		}

		m_material->setValue(name, glm::vec4(
			value[0u].GetDouble(),
			value[1u].GetDouble(),
			value[2u].GetDouble(),
			value[3u].GetDouble()));
		break;
	case ShaderParameter::kTextureType:
		if(!value.IsString()) {
			logError("%s: Expected texture for '%s'", m_path, name);
			return false;
		}

		m_material->setValue(name, g_assetManager->load<TextureBase>(value.GetString()));
		break;
	default:
		logError("%s: Cannot handle type %d for '%s'", m_path, shaderParam->type, name);
		return false;
	}

	return true;
}
