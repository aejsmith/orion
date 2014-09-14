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
Material::Material(const Shader *shader) :
	m_shader(shader),
	m_uniforms(nullptr),
	m_values(shader->numExtraParameters(), nullptr),
	m_textures(shader->numTextureParameters())
{
	if(shader->uniformStruct()) {
		/* Material parameters should be changed infrequently, therefore
		 * set the uniform buffer usage as static. */
		m_uniforms = new UniformBufferBase(*shader->uniformStruct(), GPUBuffer::kStaticDrawUsage);
	}
}

/** Destroy the material. */
Material::~Material() {
	/* Delete all parameter values. */
	for(char *value : m_values)
		delete[] value;

	delete m_uniforms;
}

/** Get a parameter value.
 * @param name		Name of the parameter to get.
 * @param type		Type of the parameter.
 * @param buf		Where to store parameter value. */
void Material::value(const char *name, ShaderParameter::Type type, void *buf) const {
	const ShaderParameter *param = m_shader->lookupParameter(name);
	orionCheck(param, "Parameter '%s' in '%s' not found", name, m_shader->name());
	orionCheck(param->type == type, "Incorrect type for parameter '%s' in '%s'", name, m_shader->name());

	switch(param->binding) {
	case ShaderParameter::kUniformBinding:
		m_uniforms->readMember(param->uniformMember, buf);
		break;
	case ShaderParameter::kTextureBinding:
		new(buf) TextureBasePtr(m_textures[param->index].second);
		break;
	default:
		if(m_values[param->index]) {
			memcpy(buf, m_values[param->index], ShaderParameter::size(param->type));
		} else {
			memset(buf, 0, ShaderParameter::size(param->type));
		}

		break;
	}
}

/** Set a parameter value.
 * @param name		Name of the parameter to set.
 * @param type		Type of the parameter.
 * @param buf		Buffer containing new parameter value. */
void Material::setValue(const char *name, ShaderParameter::Type type, const void *buf) {
	const ShaderParameter *param = m_shader->lookupParameter(name);
	orionCheck(param, "Parameter '%s' in '%s' not found", name, m_shader->name());
	orionCheck(param->type == type, "Incorrect type for parameter '%s' in '%s'", name, m_shader->name());

	switch(param->binding) {
	case ShaderParameter::kUniformBinding:
		m_uniforms->writeMember(param->uniformMember, buf);
		break;
	case ShaderParameter::kTextureBinding:
		m_textures[param->index].second = *reinterpret_cast<const TextureBasePtr *>(buf);
		break;
	default:
		if(!m_values[param->index])
			m_values[param->index] = new char[ShaderParameter::size(param->type)];

		memcpy(m_values[param->index], buf, ShaderParameter::size(param->type));
		break;
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
	const Shader *shader = Shader::lookup(shaderName);
	if(!shader) {
		orionLog(LogLevel::kError, "Material '%s' specifies unknown shader '%s'", path, shaderName);
		return nullptr;
	}

	MaterialPtr material(new Material(shader));

	/* Set all parameters. */
	const rapidjson::Value &materialParams = attributes["parameters"];
	for(auto it = materialParams.MemberBegin(); it != materialParams.MemberEnd(); ++it) {
		const ShaderParameter *shaderParam = shader->lookupParameter(it->name.GetString());
		if(!shaderParam) {
			orionLog(LogLevel::kError,
				"Material '%s' specifies unknown parameter '%s'",
				path, it->name.GetString());
			return nullptr;
		}

		const rapidjson::Value &materialParam = it->value;

		switch(shaderParam->type) {
		case ShaderParameter::kIntType:
			/* RapidJSON flags a value as being signed as long as
			 * it is within the range of a signed int. */
			if(!materialParam.IsInt()) {
				orionLog(LogLevel::kError, "Expected int for '%s' in '%s'", shaderParam->name, path);
				return nullptr;
			}

			material->setValue(shaderParam->name, materialParam.GetInt());
			break;
		case ShaderParameter::kUnsignedIntType:
			/* Similar to the above. */
			if(!materialParam.IsUint()) {
				orionLog(LogLevel::kError, "Expected uint for '%s' in '%s'", shaderParam->name, path);
				return nullptr;
			}

			material->setValue(shaderParam->name, materialParam.GetUint());
			break;
		case ShaderParameter::kFloatType:
			if(!materialParam.IsNumber()) {
				orionLog(LogLevel::kError, "Expected float for '%s' in '%s'", shaderParam->name, path);
				return nullptr;
			}

			material->setValue(shaderParam->name, static_cast<float>(materialParam.GetDouble()));
			break;
		case ShaderParameter::kVec2Type:
			if(!materialParam.IsArray() || materialParam.Size() != 2
				|| !materialParam[0u].IsNumber()
				|| !materialParam[1u].IsNumber())
			{
				orionLog(LogLevel::kError, "Expected vec2 for '%s' in '%s'", shaderParam->name, path);
				return nullptr;
			}

			material->setValue(shaderParam->name, glm::vec2(
				materialParam[0u].GetDouble(),
				materialParam[1u].GetDouble()));
			break;
		case ShaderParameter::kVec3Type:
			if(!materialParam.IsArray() || materialParam.Size() != 3
				|| !materialParam[0u].IsNumber()
				|| !materialParam[1u].IsNumber()
				|| !materialParam[2u].IsNumber())
			{
				orionLog(LogLevel::kError, "Expected vec3 for '%s' in '%s'", shaderParam->name, path);
				return nullptr;
			}

			material->setValue(shaderParam->name, glm::vec3(
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
				orionLog(LogLevel::kError, "Expected vec4 for '%s' in '%s'", shaderParam->name, path);
				return nullptr;
			}

			material->setValue(shaderParam->name, glm::vec4(
				materialParam[0u].GetDouble(),
				materialParam[1u].GetDouble(),
				materialParam[2u].GetDouble(),
				materialParam[3u].GetDouble()));
			break;
		case ShaderParameter::kTextureType:
			if(!materialParam.IsString()) {
				orionLog(LogLevel::kError, "Expected texture for '%s' in '%s'", shaderParam->name, path);
				return nullptr;
			}

			material->setValue(
				shaderParam->name,
				g_assetManager->load<TextureBase>(materialParam.GetString()));
			break;
		default:
			orionLog(LogLevel::kError,
				"MaterialLoader cannot handle type %d for '%s' in '%s'",
				shaderParam->type, shaderParam->name, path);
			return nullptr;
		}
	}

	return material;
}
