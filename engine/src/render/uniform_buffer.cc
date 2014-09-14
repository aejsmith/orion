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
const UniformStructMember *UniformStruct::lookupMember(const char *name) const {
	for(const UniformStructMember &member : this->members) {
		if(strcmp(member.name, name) == 0)
			return &member;
	}

	return nullptr;
}

/** Create the buffer, with zeroed content.
 * @param ustruct	Uniform structure type.
 * @param usage		GPU usage hint for the buffer. */
UniformBufferBase::UniformBufferBase(const UniformStruct &ustruct, GPUBuffer::Usage usage) :
	m_uniformStruct(ustruct),
	m_dirty(true)
{
	m_gpuBuffer = g_gpu->createBuffer(GPUBuffer::kUniformBuffer, usage, m_uniformStruct.size);
	m_shadowBuffer = new char[m_uniformStruct.size];
	memset(m_shadowBuffer, 0, m_uniformStruct.size);
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

/** Get the value of a member.
 * @param member	Details of the member to get.
 * @param buf		Where to store member value. */
void UniformBufferBase::readMember(const UniformStructMember *member, void *buf) const {
	memcpy(buf, m_shadowBuffer + member->offset, ShaderParameter::size(member->type));
}

/** Get the value of member.
 * @param name		Name of the member to get.
 * @param type		Expected type of the member (checked with an assertion).
 * @param buf		Where to store member value. */
void UniformBufferBase::readMember(const char *name, ShaderParameter::Type type, void *buf) const {
	const UniformStructMember *member = m_uniformStruct.lookupMember(name);

	orionCheck(member,
		"Member '%s' in uniform struct '%s' not found",
		name, m_uniformStruct.name);
	orionCheck(member->type == type,
		"Member '%s' in uniform struct '%s' incorrect type",
		name, m_uniformStruct.name);

	readMember(member, buf);
}

/** Set the value of a member.
 * @param member	Details of the member to set.
 * @param buf		Buffer containing new member value. */
void UniformBufferBase::writeMember(const UniformStructMember *member, const void *buf) const {
	m_dirty = true;
	memcpy(m_shadowBuffer + member->offset, buf, ShaderParameter::size(member->type));
}

/** Set the value of member.
 * @param name		Name of the member to set.
 * @param type		Expected type of the member (checked with an assertion).
 * @param buf		Buffer containing new member value. */
void UniformBufferBase::writeMember(const char *name, ShaderParameter::Type type, const void *buf) {
	const UniformStructMember *member = m_uniformStruct.lookupMember(name);

	orionCheck(member,
		"Member '%s' in uniform struct '%s' not found",
		name, m_uniformStruct.name);
	orionCheck(member->type == type,
		"Member '%s' in uniform struct '%s' incorrect type",
		name, m_uniformStruct.name);

	writeMember(member, buf);
}
