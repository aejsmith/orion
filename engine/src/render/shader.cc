/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Shader class.
 */

#include "engine/material.h"

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

/** Set shader-wide draw state for a material.
 * @param material	Material being rendered with. */
void Shader::setDrawState(Material *material) const {
	/* Set the uniform buffer if we have one. */
	if(m_uniformStruct) {
		UniformBufferBase *buffer = material->m_uniforms;
		orionAssert(buffer);
		g_gpu->bindUniformBuffer(UniformSlots::kMaterialUniforms, buffer->gpu());
	}

	/* Bind textures. */
	const Material::TextureArray &textures = material->m_textures;
	for(size_t i = 0; i < m_nextTextureSlot; i++) {
		if(textures[i])
			g_gpu->bindTexture(i, textures[i]->gpu());
	}
}

/** Add a parameter to the shader.
 * @param name		Name of the parameter to add.
 * @param type		Type of the parameter. */
void Shader::addParameter(const std::string &name, ShaderParameter::Type type) {
	auto ret = m_parameters.emplace(std::make_pair(name, ShaderParameter()));
	orionCheck(ret.second, "Adding duplicate shader parameter '%s'", name.c_str());

	ShaderParameter &param = ret.first->second;
	param.type = type;

	if(type == ShaderParameter::kTextureType) {
		/* Assign a texture slot. */
		orionCheck(
			m_nextTextureSlot <= TextureSlots::kMaterialTexturesEnd,
			"Parameter '%s' exceeds maximum number of textures", name.c_str());

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
 *			deleted when the shader is destroyed. */
void Shader::addPass(Pass *pass) {
	switch(pass->type()) {
	case Pass::kDeferredBasePass:
	case Pass::kDeferredOutputPass:
		orionCheck(
			m_passes[pass->type()].size() == 0,
			"Only one deferred base/output pass is allowed per shader");
		break;
	default:
		break;
	}

	/* Finalize the pipeline. */
	pass->finalize();

	m_passes[pass->type()].push_back(pass);
}
