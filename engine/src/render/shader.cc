/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Shader class.
 */

#include "render/shader.h"

/** Type of the registered shader map. */
typedef std::map<std::string, Shader *> RegisteredShaderMap;

/** @return		Registered shader map. */
static RegisteredShaderMap &registeredShaderMap() {
	static RegisteredShaderMap shaders;
	return shaders;
}

/**
 * Initialize the shader.
 *
 * Initialize the shader without a uniform structure. Derived constructors
 * should add the parameters used by the shader.
 *
 * @param name		Name of the shader.
 */
Shader::Shader(const char *name) :
	m_name(name),
	m_uniformStruct(nullptr),
	m_nextTextureIndex(0),
	m_nextExtraIndex(0)
{
	/* Register the shader. */
	RegisteredShaderMap &shaders = registeredShaderMap();
	auto ret = shaders.insert(std::make_pair(m_name, this));
	orionCheck(ret.second, "Registering shader '%s' that already exists", m_name);
}

/**
 * Initialize the shader.
 *
 * Initializes the shader. Derived constructors should add the parameters used
 * by the shader. The uniform structure parameter specifies the uniform
 * structure which will be used to pass material parameters to the GPU shaders.
 * Each material using this shader will have a uniform buffer created for it
 * that matches the layout of the specified structure.
 *
 * @param name		Name of the shader.
 * @param ustruct	Uniform structure for the shader.
 * @param bindMembers	Whether to bind all uniform struct members as shader
 *			parameters automatically (defaults to true).
 */
Shader::Shader(const char *name, const UniformStruct &ustruct, bool bindMembers) :
	Shader(name)
{
	m_uniformStruct = &ustruct;

	if(bindMembers) {
		for(const UniformStructMember &member : ustruct.members)
			addUniformParameter(member.name);
	}
}

/** Destroy the shader. */
Shader::~Shader() {
	/* Unregister the shader. */
	RegisteredShaderMap &shaders = registeredShaderMap();
	shaders.erase(m_name);
}

/** Look up a shader by name.
 * @param name		Name of the shader to look up.
 * @return		Pointer to shader if found, null if not. */
const Shader *Shader::lookup(const std::string &name) {
	RegisteredShaderMap &shaders = registeredShaderMap();
	auto ret = shaders.find(name);
	return (ret != shaders.end()) ? ret->second : nullptr;
}

/**
 * Parameter management.
 */

/** Add a parameter to the parameter list.
 * @param param		Parameter to add. */
void Shader::addParameter(const ShaderParameter &param) {
	auto ret = m_parameters.insert(std::make_pair(param.name, param));
	orionCheck(ret.second, "Adding duplicate parameter '%s' to '%s'", param.name, m_name);
}

/**
 * Add a uniform parameter.
 *
 * Adds a uniform parameter to the shader. Uniform parameters are bound to
 * a member in the shader's uniform structure, and have the type of the
 * specified member. The specified member will automatically be set in a
 * material's uniform buffer when the parameter is changed in the material.
 *
 * @param name		Name of the parameter to add.
 * @param memberName	Uniform structure member name to bind to. If null (the
 *			default), the same value specified for name will be
 *			used.
 */
void Shader::addUniformParameter(const char *name, const char *memberName) {
	orionCheck(m_uniformStruct, "Adding uniform parameter '%s' to '%s' without struct", name, m_name);

	if(!memberName)
		memberName = name;

	const UniformStructMember *member = m_uniformStruct->lookupMember(memberName);
	orionCheck(member, "Unknown uniform struct member '%s' in '%s'", memberName, m_name);

	ShaderParameter param;
	param.name = name;
	param.type = member->type;
	param.binding = ShaderParameter::kUniformBinding;
	param.index = 0;
	param.uniformMember = member;
	addParameter(param);
}

/**
 * Add a texture parameter.
 *
 * Adds a texture parameter to the shader. Texture parameters are automatically
 * bound to the specified texture slot when rendering takes place using the
 * shader.
 *
 * @param name		Name of the parameter to add.
 * @param slot		Texture slot to bind to.
 */
void Shader::addTextureParameter(const char *name, unsigned slot) {
	ShaderParameter param;
	param.name = name;
	param.type = ShaderParameter::kTextureType;
	param.binding = ShaderParameter::kTextureBinding;
	param.index = m_nextTextureIndex++;
	param.textureSlot = slot;
	addParameter(param);
}

/**
 * Add an extra parameter to the shader.
 *
 * Adds an extra parameter to the shader. Extra parameter values are not
 * automatically copied to a uniform buffer or bound to a texture slot, their
 * usage is entirely up to C++ shader code.
 *
 * @param name		Name of the parameter to add.
 * @param type		Type of the parameter.
 */
void Shader::addExtraParameter(const char *name, ShaderParameter::Type type) {
	/* Requires a bit of awkwardness to store a TexturePtr in the
	 * Material::Value union, so I haven't bothered adding it until I need
	 * it, if ever. */
	orionCheck(type != ShaderParameter::kTextureType, "Extra texture parameters are not supported");

	ShaderParameter param;
	param.name = name;
	param.type = type;
	param.binding = ShaderParameter::kNoBinding;
	param.index = m_nextExtraIndex++;
	addParameter(param);
}

/** Look up a parameter by name.
 * @param name		Name of the parameter to look up.
 * @return		Pointer to parameter if found, null if not. */
const ShaderParameter *Shader::lookupParameter(const std::string &name) const {
	auto it = m_parameters.find(name);
	return (it != m_parameters.end()) ? &it->second : nullptr;
}
