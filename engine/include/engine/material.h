/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Material class.
 */

#pragma once

#include "engine/asset.h"
#include "engine/texture.h"

#include "render/defs.h"

#include <vector>

class Shader;
class UniformBufferBase;

/**
 * Material class.
 *
 * A material is applied to a mesh and defines the how the mesh looks when it
 * is rendered. It holds a reference to a shader and a set of parameters to the
 * shader.
 */
class Material : public Asset {
public:
	explicit Material(const Shader *shader);
	~Material();

	/** @return		Shader for the material. */
	const Shader *shader() const { return m_shader; }

	/**
	 * Parameter value access.
	 */

	void value(const char *name, ShaderParameter::Type type, void *buf) const;
	void setValue(const char *name, ShaderParameter::Type type, const void *buf);

	/** Get a parameter value.
	 * @tparam T		Type of the parameter.
	 * @param name		Name of the parameter to get.
	 * @return 		Parameter value. */
	template <typename T> T value(const char *name) {
		T ret;
		value(name, ShaderParameterTypeTraits<T>::kType, std::addressof(ret));
		return ret;
	}

	/** Set a parameter value.
	 * @tparam T		Type of the parameter.
	 * @param name		Name of the parameter to set.
	 * @param value		Value to set to. */
	template <typename T> void setValue(const char *name, const T &value) {
		setValue(name, ShaderParameterTypeTraits<T>::kType, std::addressof(value));
	}
private:
	const Shader *m_shader;		/**< Shader being used by the material. */
	UniformBufferBase *m_uniforms;	/**< Uniform buffer containing material parameters. */

	/** Extra parameter values (indexed by ShaderParameter::index). */
	std::vector<char *> m_values;

	/**
	 * Textures.
	 *
	 * Array of textures for the material, indexed by ShaderParameter::index.
	 * We additionally store the source parameter structure to access its
	 * binding information.
	 */
	std::vector<std::pair<const ShaderParameter *, TextureBasePtr>> m_textures;
};

/** Type of a material pointer. */
typedef TypedAssetPtr<Material> MaterialPtr;
