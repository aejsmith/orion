/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Material class.
 */

#include "engine/material.h"

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
	checkMsg(param, "Parameter '%s' in '%s' not found", name, m_shader->path().c_str());
	checkMsg(param->type == type, "Incorrect type for parameter '%s' in '%s'", name, m_shader->path().c_str());

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
	checkMsg(param, "Parameter '%s' in '%s' not found", name, m_shader->path().c_str());
	checkMsg(param->type == type, "Incorrect type for parameter '%s' in '%s'", name, m_shader->path().c_str());

	if(param->type == ShaderParameter::kTextureType) {
		m_textures[param->textureSlot] = *reinterpret_cast<const TextureBasePtr *>(buf);
	} else {
		m_uniforms->writeMember(param->uniformMember, buf);
	}
}
