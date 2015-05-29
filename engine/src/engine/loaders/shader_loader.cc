/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Shader asset loader.
 */

#include "engine/asset_loader.h"

#include "render/shader.h"

/** Asset loader for shaders. */
class ShaderLoader : public AssetLoader {
public:
    bool dataIsMetadata() const override { return true; }
    AssetPtr load() override;
private:
    bool addParameter(const char *name, const rapidjson::Value &value);
    bool addPass(const rapidjson::Value &desc);

    bool loadStage(Pass *pass, GPUShader::Type stage, const rapidjson::Value &value);
private:
    ShaderPtr m_shader;
};

IMPLEMENT_ASSET_LOADER(ShaderLoader, "osh");

/** Load a shader asset.
 * @return              Pointer to loaded asset, null on failure. */
AssetPtr ShaderLoader::load() {
    m_shader = new Shader();

    /* Add parameters if there are any. */
    if (m_attributes.HasMember("parameters")) {
        if (!m_attributes["parameters"].IsObject()) {
            logError("%s: 'parameters' attribute should be an object", m_path);
            return nullptr;
        }

        const rapidjson::Value &params = m_attributes["parameters"];
        for (auto it = params.MemberBegin(); it != params.MemberEnd(); ++it) {
            if (!addParameter(it->name.GetString(), it->value))
                return nullptr;
        }
    }

    if (!m_attributes.HasMember("passes")) {
        logError("%s: 'passes' attribute is missing", m_path);
        return nullptr;
    } else if (!m_attributes["passes"].IsArray()) {
        logError("%s: 'passes' attribute should be an array", m_path);
        return nullptr;
    }

    /* Add passes. */
    const rapidjson::Value &passes = m_attributes["passes"];
    for (auto it = passes.Begin(); it != passes.End(); ++it) {
        if (!addPass(*it))
            return nullptr;
    }

    return m_shader;
}

/** Add a shader parameter.
 * @param name          Name of the parameter.
 * @param desc          Parameter descriptor.
 * @return              Whether the parameter was successfully added. */
bool ShaderLoader::addParameter(const char *name, const rapidjson::Value &desc) {
    if (strlen(name) == 0) {
        logError("%s: Parameter name is empty", m_path);
        return false;
    } else if (!desc.IsObject()) {
        logError("%s: Parameter '%s' descriptor should be an object", m_path, name);
        return false;
    } else if (!desc.HasMember("type") || !desc["type"].IsString()) {
        logError("%s: Parameter '%s' 'type' attribute is missing/not a string", m_path, name);
        return false;
    }

    const char *typeString = desc["type"].GetString();
    ShaderParameter::Type type;
    unsigned textureSlot = -1u;
    if (strcmp(typeString, "Int") == 0) {
        type = ShaderParameter::kIntType;
    } else if (strcmp(typeString, "UnsignedInt") == 0) {
        type = ShaderParameter::kUnsignedIntType;
    } else if (strcmp(typeString, "Float") == 0) {
        type = ShaderParameter::kFloatType;
    } else if (strcmp(typeString, "Vec2") == 0) {
        type = ShaderParameter::kVec2Type;
    } else if (strcmp(typeString, "Vec3") == 0) {
        type = ShaderParameter::kVec3Type;
    } else if (strcmp(typeString, "Vec4") == 0) {
        type = ShaderParameter::kVec4Type;
    } else if (strcmp(typeString, "Mat2") == 0) {
        type = ShaderParameter::kMat2Type;
    } else if (strcmp(typeString, "Mat3") == 0) {
        type = ShaderParameter::kMat3Type;
    } else if (strcmp(typeString, "Mat4") == 0) {
        type = ShaderParameter::kMat4Type;
    } else if (strcmp(typeString, "Texture") == 0) {
        type = ShaderParameter::kTextureType;

        /* Check if a slot is specified. */
        if (desc.HasMember("slot")) {
            if (!desc["slot"].IsUint()) {
                logError("%s: Parameter '%s' 'slot' attribute is not an integer", m_path, name);
                return false;
            }

            textureSlot = desc["slot"].GetUint();
            if (textureSlot <= TextureSlots::kMaterialTexturesEnd) {
                logError("%s: Parameter '%s' texture slot is not valid", m_path, name);
                return false;
            }
        } else if (m_shader->numTextures() > TextureSlots::kMaterialTexturesEnd) {
            logError("%s: Maximum number of textures exceeded", m_path);
            return false;
        }
    } else {
        logError("%s: Parameter '%s' type '%s' is invalid", m_path, name, typeString);
        return false;
    }

    if (m_shader->lookupParameter(name)) {
        logError("%s: Duplicate parameter '%s'", m_path, name);
        return false;
    }

    m_shader->addParameter(name, type, textureSlot);
    return true;
}

/** Add a shader pass.
 * @param desc          Pass descriptor.
 * @return              Whether the pass was successfully added. */
bool ShaderLoader::addPass(const rapidjson::Value &desc) {
    if (!desc.IsObject()) {
        logError("%s: Pass descriptor should be an object", m_path);
        return false;
    } else if (!desc.HasMember("type") || !desc["type"].IsString()) {
        logError("%s: Pass 'type' attribute is missing/not a string", m_path);
        return false;
    }

    const char *typeString = desc["type"].GetString();
    Pass::Type type;
    if (strcmp(typeString, "Basic") == 0) {
        type = Pass::kBasicPass;
    } else if (strcmp(typeString, "Forward") == 0) {
        type = Pass::kForwardPass;
    } else if (strcmp(typeString, "Deferred") == 0) {
        type = Pass::kDeferredPass;
    } else {
        logError("%s: Pass type '%s' is invalid", m_path, typeString);
        return false;
    }

    switch (type) {
        case Pass::kDeferredPass:
            if (m_shader->numPasses(type)) {
                logError("%s: Only one pass of type '%s' allowed per shader", m_path, typeString);
                return false;
            }
        default:
            break;
    }

    std::unique_ptr<Pass> pass(new Pass(m_shader.get(), type));

    if (!desc.HasMember("vertex") || !desc.HasMember("fragment")) {
        logError("%s: Pass requires at least vertex and fragment shaders", m_path);
        return false;
    }

    if (!loadStage(pass.get(), GPUShader::kVertexShader, desc["vertex"]))
        return false;
    if (!loadStage(pass.get(), GPUShader::kFragmentShader, desc["fragment"]))
        return false;

    m_shader->addPass(pass.release());
    return true;
}

/** Load a stage in a pass.
 * @param pass          Pass to load into.
 * @param stage         Type of the stage to load.
 * @param value         Value containing path string. */
bool ShaderLoader::loadStage(Pass *pass, GPUShader::Type stage, const rapidjson::Value &value) {
    if (!value.IsObject()) {
        logError("%s: Pass stage should be an object", m_path);
        return false;
    } else if (!value.HasMember("source") || !value["source"].IsString()) {
        logError("%s: Pass stage 'source' attribute is missing/not a string", m_path);
        return false;
    }

    Path path(value["source"].GetString());

    /* Get keywords. */
    Pass::KeywordSet keywords;
    if (value.HasMember("keywords")) {
        const rapidjson::Value &keywordsValue = value["keywords"];

        if (!keywordsValue.IsArray()) {
            logError("%s: Pass stage 'keywords' attribute is not an array", m_path);
            return false;
        }

        for (auto it = keywordsValue.Begin(); it != keywordsValue.End(); ++it) {
            if (!it->IsString()) {
                logError("%s: Expected string for keyword name", m_path);
                return false;
            }

            keywords.insert(it->GetString());
        }
    }

    return pass->loadStage(stage, path, keywords);
}
