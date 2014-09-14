/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Shader classes.
 */

#pragma once

#include "engine/texture.h"

#include "render/uniform_buffer.h"

#include <map>

/**
 * Shader class
 *
 * This class implements the CPU side of a shader. A shader defines a set of
 * parameters, and a set of rendering passes required to achieve the desired
 * effect. A pass defines the actual GPU shaders that will be used. Materials
 * are assets which supply parameters to shaders.
 *
 * There are 3 categories of shader parameters: uniforms, textures, and extra
 * parameters. Uniform parameters are ones which are passed directly to the
 * GPU shader code, these are automatically filled into the material's uniform
 * buffer. Texture parameters (obviously) specify a texture used by the shader,
 * these are specified with a texture slot index that the texture will be bound
 * to. Finally, extra parameters are ones used by the C++ shader code. Their
 * values are stored in Material and can be retrieved with Material::getValue().
 */
class Shader : Noncopyable {
public:
	/** Type of the parameter map. */
	typedef std::map<std::string, ShaderParameter> ParameterMap;
public:
	virtual ~Shader();

	/** @return		Name of the shader. */
	const char *name() const { return m_name; }
	/** @return		Uniform structure used by the shader. */
	const UniformStruct *uniformStruct() const { return m_uniformStruct; }
	/** @return		Parameter map for the shader. */
	const ParameterMap &parameters() const { return m_parameters; }
	/** @return		Number of texture parameters. */
	unsigned numTextureParameters() const { return m_nextTextureIndex; }
	/** @return		Number of extra parameters. */
	unsigned numExtraParameters() const { return m_nextExtraIndex; }

	const ShaderParameter *lookupParameter(const std::string &name) const;

	static const Shader *lookup(const std::string &name);
protected:
	Shader(const char *name);
	Shader(const char *name, const UniformStruct &ustruct, bool bindMembers = true);

	void addUniformParameter(const char *name, const char *memberName = nullptr);
	void addTextureParameter(const char *name, unsigned slot);
	void addExtraParameter(const char *name, ShaderParameter::Type type);

	/** Add an extra parameter to the shader.
	 * @tparam T		Type of the parameter.
	 * @param name		Name of the parameter. */
	template <typename T> void addExtraParameter(const char *name) {
		addExtraParameter(name, ShaderParameterTypeTraits<T>::kType);
	}
private:
	void addParameter(const ShaderParameter &param);
private:
	const char *m_name;			/**< Name of the shader. */
	const UniformStruct *m_uniformStruct;	/**< Uniform structure used by the shader. */
	ParameterMap m_parameters;		/**< Map of registered parameters. */
	unsigned m_nextTextureIndex;		/**< Next texture parameter index. */
	unsigned m_nextExtraIndex;		/**< Next extra parameter index. */
};
