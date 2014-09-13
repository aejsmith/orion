/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Uniform buffer classes.
 *
 * @todo		Can we optimize updates of buffers by determining the
 *			region within the buffer that's dirty somehow and only
 *			upload that? I'm not sure how much benefit that'll be.
 *			Measure it!
 */

#include "render/uniform_buffer.h"

/** Find a member in the structure by name.
 * @param name		Name of the member to find.
 * @return		Pointer to member if found, null if not. */
const UniformStruct::Member *UniformStruct::lookupMember(const char *name) const {
	for(const Member &member : this->members) {
		if(strcmp(member.name, name) == 0)
			return &member;
	}

	return nullptr;
}

/** Create the buffer, with undefined content.
 * @param ustruct	Uniform structure type.
 * @param usage		GPU usage hint for the buffer. */
UniformBufferBase::UniformBufferBase(const UniformStruct &ustruct, GPUBuffer::Usage usage) :
	m_uniformStruct(ustruct),
	m_dirty(true)
{
	m_gpuBuffer = g_gpu->createBuffer(GPUBuffer::kUniformBuffer, usage, m_uniformStruct.size);
	m_shadowBuffer = new char[m_uniformStruct.size];
}

/** Destroy the buffer. */
UniformBufferBase::~UniformBufferBase() {
	delete[] m_shadowBuffer;
}

/**
 * Retrieve the GPU buffer.
 *
 * Retrieve the GPU buffer. Any modifications made to the buffer content on the
 * CPU side since the last call to this function will be uploaded to the GPU
 * buffer.
 *
 * @return		Pointer to GPU buffer.
 */
GPUBufferPtr UniformBufferBase::gpu() const {
	if(m_dirty) {
		m_gpuBuffer->write(0, m_uniformStruct.size, m_shadowBuffer);
		m_dirty = false;
	}

	return m_gpuBuffer;
}

/** Get the size of a shader parameter type.
 * @param type		Type to get size of.
 * @return		Size of the type. */
static inline size_t shaderParameterSize(ShaderParameterType type) {
	switch(type) {
	case ShaderParameterType::kInt:
		return sizeof(int);
	case ShaderParameterType::kUnsignedInt:
		return sizeof(unsigned int);
	case ShaderParameterType::kFloat:
		return sizeof(float);
	case ShaderParameterType::kVec2:
		return sizeof(glm::vec2);
	case ShaderParameterType::kVec3:
		return sizeof(glm::vec3);
	case ShaderParameterType::kVec4:
		return sizeof(glm::vec4);
	case ShaderParameterType::kMat2:
		return sizeof(glm::mat2);
	case ShaderParameterType::kMat3:
		return sizeof(glm::mat3);
	case ShaderParameterType::kMat4:
		return sizeof(glm::mat4);
	default:
		orionAbort("Invalid parameter type %d passed to {get,set}Member", type);
	}
}

/** Get the value of a member.
 * @param member	Details of the member to get.
 * @param value		Where to store member value. */
void UniformBufferBase::getMember(const UniformStruct::Member *member, void *value) const {
	memcpy(value, m_shadowBuffer + member->offset, shaderParameterSize(member->type));
}

/** Get the value of member.
 * @param name		Name of the member to get.
 * @param type		Expected type of the member (checked with an assertion).
 * @param value		Where to store member value. */
void UniformBufferBase::getMember(const char *name, ShaderParameterType type, void *value) const {
	const UniformStruct::Member *member = m_uniformStruct.lookupMember(name);
	orionCheck(member, "Member '%s' in uniform struct '%s' not found", name, m_uniformStruct.name);
	orionCheck(member->type == type, "Member '%s' in uniform struct '%s' incorrect type", name, m_uniformStruct.name);

	getMember(member, value);
}

/** Set the value of a member.
 * @param member	Details of the member to set.
 * @param value		Buffer containing new member value. */
void UniformBufferBase::setMember(const UniformStruct::Member *member, const void *value) const {
	m_dirty = true;
	memcpy(m_shadowBuffer + member->offset, value, shaderParameterSize(member->type));
}

/** Set the value of member.
 * @param name		Name of the member to set.
 * @param type		Expected type of the member (checked with an assertion).
 * @param value		Buffer containing new member value. */
void UniformBufferBase::setMember(const char *name, ShaderParameterType type, const void *value) {
	const UniformStruct::Member *member = m_uniformStruct.lookupMember(name);
	orionCheck(member, "Member '%s' in uniform struct '%s' not found", name, m_uniformStruct.name);
	orionCheck(member->type == type, "Member '%s' in uniform struct '%s' incorrect type", name, m_uniformStruct.name);

	setMember(member, value);
}
