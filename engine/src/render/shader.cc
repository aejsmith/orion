/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Shader class.
 */

#include "engine/asset_loader.h"

#include "render/pass.h"
#include "render/shader.h"

/** Initialize the shader. */
Shader::Shader() :
	m_uniformStruct(nullptr),
	m_nextTextureSlot(0)
{}

/** Destroy the shader. */
Shader::~Shader() {
	/* Delete all passes. */
	for(size_t i = 0; i < m_passes.size(); i++) {
		for(size_t j = 0; j < m_passes[i].size(); j++)
			delete m_passes[i][j];
	}

	delete m_uniformStruct;
}

/** Add a parameter to the shader.
 * @param name		Name of the parameter to add.
 * @param type		Type of the parameter.
 * @return		Whether added successfully. */
bool Shader::addParameter(const std::string &name, ShaderParameter::Type type) {
	auto ret = m_parameters.emplace(std::make_pair(name, ShaderParameter()));
	if(!ret.second) {
		orionLog(LogLevel::kError, "Adding duplicate parameter '%s'", name.c_str());
		return false;
	}

	ShaderParameter &param = ret.first->second;
	param.type = type;

	if(type == ShaderParameter::kTextureType) {
		/* Assign a texture slot. */
		if(m_nextTextureSlot > TextureSlots::kMaterialTexturesEnd) {
			orionLog(LogLevel::kError, "Parameter '%s' exceeds maximum number of textures", name.c_str());
			return false;
		}

		param.textureSlot = m_nextTextureSlot++;
	} else {
		/* Add a uniform struct member for it. Create struct if we don't
		 * already have one. */
		if(!m_uniformStruct)
			m_uniformStruct = new UniformStruct("MaterialUniforms", nullptr, UniformSlots::kMaterialUniforms);

		/* A bit nasty, UniformStructMember has a char * for name, not a
		 * std::string, so we point to the name string in the map key.
		 * This avoids storing multiple copies of the name string. */
		param.uniformMember = m_uniformStruct->addMember(ret.first->first.c_str(), type);
	}

	return true;
}

/** Look up a parameter by name.
 * @param name		Name of the parameter to look up.
 * @return		Pointer to parameter if found, null if not. */
const ShaderParameter *Shader::lookupParameter(const std::string &name) const {
	auto it = m_parameters.find(name);
	return (it != m_parameters.end()) ? &it->second : nullptr;
}

/** Add a pass to the shader.
 * @param pass		Pass to add. Becomes owned by the shader, will be
 *			deleted when the shader is destroyed.
 * @return		Whether the pass was added successfully. */
bool Shader::addPass(Pass *pass) {
	switch(pass->type()) {
	case Pass::kDeferredBasePass:
	case Pass::kDeferredOutputPass:
		if(m_passes[pass->type()].size() != 0) {
			orionLog(LogLevel::kError, "Only one deferred base/output pass is allowed per shader");
			return false;
		}

		break;
	default:
		break;
	}

	/* Finalize the pipeline. */
	pass->finalize();

	m_passes[pass->type()].push_back(pass);
	return true;
}

/**
 * Shader asset loader.
 */

/** Asset loader for shaders. */
class ShaderLoader : public AssetLoader {
public:
	ShaderLoader() : AssetLoader("osh") {}
	bool dataIsMetadata() const override { return true; }
	AssetPtr load(DataStream *stream, rapidjson::Value &attributes, const char *path) const override;
private:
	static bool convertParameterType(const rapidjson::Value &value, ShaderParameter::Type &type);
	static bool convertPassType(const rapidjson::Value &value, Pass::Type &type);
	static bool loadStage(Pass *pass, GPUShader::Type stage, const rapidjson::Value &value);

	static ShaderLoader m_instance;
};

/** Shader loader instance. */
ShaderLoader ShaderLoader::m_instance;

/** Load a shader asset.
 * @param stream	Stream containing asset data.
 * @param attributes	Attributes specified in metadata.
 * @param path		Path to asset.
 * @return		Pointer to loaded asset, null on failure. */
AssetPtr ShaderLoader::load(DataStream *stream, rapidjson::Value &attributes, const char *path) const {
	if(attributes.HasMember("parameters") && !attributes["parameters"].IsObject()) {
		orionLog(LogLevel::kError, "Shader 'parameters' attribute is invalid");
		return nullptr;
	} else if(!attributes.HasMember("passes") || !attributes["passes"].IsArray()) {
		orionLog(LogLevel::kError, "Shader 'passes' attribute is missing/invalid");
		return nullptr;
	}

	ShaderPtr shader(new Shader());

	/* Add parameters if there are any. */
	if(attributes.HasMember("parameters")) {
		const rapidjson::Value &params = attributes["parameters"];
		for(auto it = params.MemberBegin(); it != params.MemberEnd(); ++it) {
			const char *paramName = it->name.GetString();
			if(strlen(paramName) == 0) {
				orionLog(LogLevel::kError, "Shader parameter name is empty");
				return nullptr;
			}

			ShaderParameter::Type paramType;
			if(!convertParameterType(it->value, paramType))
				return nullptr;

			if(!shader->addParameter(paramName, paramType))
				return nullptr;
		}
	}

	/* Add passes. */
	const rapidjson::Value &passes = attributes["passes"];
	for(auto it = passes.Begin(); it != passes.End(); ++it) {
		const rapidjson::Value &passDesc = *it;
		if(!passDesc.IsObject()) {
			orionLog(LogLevel::kError, "Shader pass descriptor must be an object");
			return nullptr;
		} else if(!passDesc.HasMember("type")) {
			orionLog(LogLevel::kError, "Shader pass type is missing");
			return nullptr;
		}

		Pass::Type passType;
		if(!convertPassType(passDesc["type"], passType))
			return nullptr;

		std::unique_ptr<Pass> pass(new Pass(shader.get(), passType));

		if(!passDesc.HasMember("vertex") || !passDesc.HasMember("fragment")) {
			orionLog(LogLevel::kError, "Shader pass requires at least a vertex and fragment shader");
			return nullptr;
		}

		if(!loadStage(pass.get(), GPUShader::kVertexShader, passDesc["vertex"]))
			return nullptr;
		if(!loadStage(pass.get(), GPUShader::kFragmentShader, passDesc["fragment"]))
			return nullptr;

		shader->addPass(pass.release());
	}

	return shader;
}

/** Convert a parameter type string.
 * @param value		Parameter type value.
 * @param type		Where to store converted type.
 * @return		Whether the type was converted successfully. */
bool ShaderLoader::convertParameterType(const rapidjson::Value &value, ShaderParameter::Type &type) {
	if(!value.IsString()) {
		orionLog(LogLevel::kError, "Shader parameter type should be a string");
		return false;
	}

	if(strcmp(value.GetString(), "Int") == 0) {
		type = ShaderParameter::kIntType;
		return true;
	} else if(strcmp(value.GetString(), "UnsignedInt") == 0) {
		type = ShaderParameter::kUnsignedIntType;
		return true;
	} else if(strcmp(value.GetString(), "Float") == 0) {
		type = ShaderParameter::kFloatType;
		return true;
	} else if(strcmp(value.GetString(), "Vec2") == 0) {
		type = ShaderParameter::kVec2Type;
		return true;
	} else if(strcmp(value.GetString(), "Vec3") == 0) {
		type = ShaderParameter::kVec3Type;
		return true;
	} else if(strcmp(value.GetString(), "Vec4") == 0) {
		type = ShaderParameter::kVec4Type;
		return true;
	} else if(strcmp(value.GetString(), "Mat2") == 0) {
		type = ShaderParameter::kMat2Type;
		return true;
	} else if(strcmp(value.GetString(), "Mat3") == 0) {
		type = ShaderParameter::kMat3Type;
		return true;
	} else if(strcmp(value.GetString(), "Mat4") == 0) {
		type = ShaderParameter::kMat4Type;
		return true;
	} else if(strcmp(value.GetString(), "Texture") == 0) {
		type = ShaderParameter::kTextureType;
		return true;
	}

	orionLog(LogLevel::kError, "Shader parameter type '%s' is invalid", value.GetString());
	return false;
}

/** Convert a pass type string.
 * @param value		Pass type value.
 * @param type		Where to store converted type.
 * @return		Whether the type was converted successfully. */
bool ShaderLoader::convertPassType(const rapidjson::Value &value, Pass::Type &type) {
	if(!value.IsString()) {
		orionLog(LogLevel::kError, "Shader pass type should be a string");
		return false;
	}

	if(strcmp(value.GetString(), "Basic") == 0) {
		type = Pass::kBasicPass;
		return true;
	} else if(strcmp(value.GetString(), "Forward") == 0) {
		type = Pass::kForwardPass;
		return true;
	} else if(strcmp(value.GetString(), "DeferredBase") == 0) {
		type = Pass::kDeferredBasePass;
		return true;
	} else if(strcmp(value.GetString(), "DeferredOutput") == 0) {
		type = Pass::kDeferredOutputPass;
		return true;
	}

	orionLog(LogLevel::kError, "Shader pass type '%s' is invalid", value.GetString());
	return false;
}

/** Load a stage in a pass.
 * @param pass		Pass to load into.
 * @param stage		Type of the stage to load.
 * @param value		Value containing path string. */
bool ShaderLoader::loadStage(Pass *pass, GPUShader::Type stage, const rapidjson::Value &value) {
	if(!value.IsString()) {
		orionLog(LogLevel::kError, "Shader pass stage should specify a path string");
		return false;
	}

	return pass->loadStage(stage, value.GetString());
}
