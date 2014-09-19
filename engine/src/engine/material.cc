/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Material class.
 */

#include "engine/asset_loader.h"
#include "engine/asset_manager.h"
#include "engine/material.h"

#include "render/shader.h"

/** Create a new material.
 * @param shader	Shader to use for the material. */
Material::Material(ShaderPtr shader) :
	m_shader(shader),
	m_uniforms(nullptr)
{
	if(shader->uniformStruct()) {
		/* Material parameters should be changed infrequently, therefore
		 * set the uniform buffer usage as static. */
		m_uniforms = new UniformBufferBase(*shader->uniformStruct(), GPUBuffer::kStaticDrawUsage);
	}
}

/** Destroy the material. */
Material::~Material() {
	delete m_uniforms;
}

/** Get a parameter value.
 * @param name		Name of the parameter to get.
 * @param type		Type of the parameter.
 * @param buf		Where to store parameter value. */
void Material::value(const char *name, ShaderParameter::Type type, void *buf) const {
	const ShaderParameter *param = m_shader->lookupParameter(name);
	orionCheck(param, "Parameter '%s' in '%s' not found", name, m_shader->path().c_str());
	orionCheck(param->type == type, "Incorrect type for parameter '%s' in '%s'", name, m_shader->path().c_str());

	if(param->type == ShaderParameter::kTextureType) {
		new(buf) TextureBasePtr(m_textures[param->textureSlot]);
	} else {
		m_uniforms->readMember(param->uniformMember, buf);
	}
}

/** Set a parameter value.
 * @param name		Name of the parameter to set.
 * @param type		Type of the parameter.
 * @param buf		Buffer containing new parameter value. */
void Material::setValue(const char *name, ShaderParameter::Type type, const void *buf) {
	const ShaderParameter *param = m_shader->lookupParameter(name);
	orionCheck(param, "Parameter '%s' in '%s' not found", name, m_shader->path().c_str());
	orionCheck(param->type == type, "Incorrect type for parameter '%s' in '%s'", name, m_shader->path().c_str());

	if(param->type == ShaderParameter::kTextureType) {
		m_textures[param->textureSlot] = *reinterpret_cast<const TextureBasePtr *>(buf);
	} else {
		m_uniforms->writeMember(param->uniformMember, buf);
	}
}

/**
 * Material asset loader.
 */

/** Asset loader for materials. */
class MaterialLoader : public AssetLoader {
public:
	MaterialLoader() : AssetLoader("omt") {}
	bool dataIsMetadata() const override { return true; }
	AssetPtr load(DataStream *stream, rapidjson::Value &attributes, const char *path) const override;
private:
	static MaterialLoader m_instance;
};

/** Material loader instance. */
MaterialLoader MaterialLoader::m_instance;

/** Load a material.
 * @param stream	Stream containing asset data.
 * @param attributes	Attributes specified in metadata.
 * @param path		Path to asset.
 * @return		Pointer to loaded asset, null on failure. */
AssetPtr MaterialLoader::load(DataStream *stream, rapidjson::Value &attributes, const char *path) const {
	if(!attributes.HasMember("shader") || !attributes["shader"].IsString()) {
		orionLog(LogLevel::kError, "Material '%s' specifies no shader", path);
		return nullptr;
	} else if(!attributes.HasMember("parameters") || !attributes["parameters"].IsObject()) {
		orionLog(LogLevel::kError, "Material '%s' specifies no parameters", path);
		return nullptr;
	}

	const char *shaderName = attributes["shader"].GetString();
	ShaderPtr shader = g_assetManager->load<Shader>(shaderName);
	if(!shader) {
		// FIXME: asset error handling.
		orionLog(LogLevel::kError, "Material '%s' specifies unknown shader '%s'", path, shaderName);
		return nullptr;
	}

	MaterialPtr material(new Material(shader));

	/* Set all parameters. */
	const rapidjson::Value &materialParams = attributes["parameters"];
	for(auto it = materialParams.MemberBegin(); it != materialParams.MemberEnd(); ++it) {
		const char *paramName = it->name.GetString();
		const ShaderParameter *shaderParam = shader->lookupParameter(paramName);
		if(!shaderParam) {
			orionLog(LogLevel::kError,
				"Material '%s' specifies unknown parameter '%s'",
				path, paramName);
			return nullptr;
		}

		const rapidjson::Value &materialParam = it->value;

		switch(shaderParam->type) {
		case ShaderParameter::kIntType:
			/* RapidJSON flags a value as being signed as long as
			 * it is within the range of a signed int. */
			if(!materialParam.IsInt()) {
				orionLog(LogLevel::kError, "Expected int for '%s' in '%s'", paramName, path);
				return nullptr;
			}

			material->setValue(paramName, materialParam.GetInt());
			break;
		case ShaderParameter::kUnsignedIntType:
			/* Similar to the above. */
			if(!materialParam.IsUint()) {
				orionLog(LogLevel::kError, "Expected uint for '%s' in '%s'", paramName, path);
				return nullptr;
			}

			material->setValue(paramName, materialParam.GetUint());
			break;
		case ShaderParameter::kFloatType:
			if(!materialParam.IsNumber()) {
				orionLog(LogLevel::kError, "Expected float for '%s' in '%s'", paramName, path);
				return nullptr;
			}

			material->setValue(paramName, static_cast<float>(materialParam.GetDouble()));
			break;
		case ShaderParameter::kVec2Type:
			if(!materialParam.IsArray() || materialParam.Size() != 2
				|| !materialParam[0u].IsNumber()
				|| !materialParam[1u].IsNumber())
			{
				orionLog(LogLevel::kError, "Expected vec2 for '%s' in '%s'", paramName, path);
				return nullptr;
			}

			material->setValue(paramName, glm::vec2(
				materialParam[0u].GetDouble(),
				materialParam[1u].GetDouble()));
			break;
		case ShaderParameter::kVec3Type:
			if(!materialParam.IsArray() || materialParam.Size() != 3
				|| !materialParam[0u].IsNumber()
				|| !materialParam[1u].IsNumber()
				|| !materialParam[2u].IsNumber())
			{
				orionLog(LogLevel::kError, "Expected vec3 for '%s' in '%s'", paramName, path);
				return nullptr;
			}

			material->setValue(paramName, glm::vec3(
				materialParam[0u].GetDouble(),
				materialParam[1u].GetDouble(),
				materialParam[2u].GetDouble()));
			break;
		case ShaderParameter::kVec4Type:
			if(!materialParam.IsArray() || materialParam.Size() != 4
				|| !materialParam[0u].IsNumber()
				|| !materialParam[1u].IsNumber()
				|| !materialParam[2u].IsNumber()
				|| !materialParam[3u].IsNumber())
			{
				orionLog(LogLevel::kError, "Expected vec4 for '%s' in '%s'", paramName, path);
				return nullptr;
			}

			material->setValue(paramName, glm::vec4(
				materialParam[0u].GetDouble(),
				materialParam[1u].GetDouble(),
				materialParam[2u].GetDouble(),
				materialParam[3u].GetDouble()));
			break;
		case ShaderParameter::kTextureType:
			if(!materialParam.IsString()) {
				orionLog(LogLevel::kError, "Expected texture for '%s' in '%s'", paramName, path);
				return nullptr;
			}

			material->setValue(
				paramName,
				g_assetManager->load<TextureBase>(materialParam.GetString()));
			break;
		default:
			orionLog(LogLevel::kError,
				"MaterialLoader cannot handle type %d for '%s' in '%s'",
				shaderParam->type, paramName, path);
			return nullptr;
		}
	}

	return material;
}
