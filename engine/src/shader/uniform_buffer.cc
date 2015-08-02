/*
 * Copyright (C) 2015 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               Uniform buffer classes.
 *
 * TODO:
 *  - Can we optimize updates of buffers by determining the region within the
 *    buffer that's dirty somehow and only upload that? I'm not sure how much
 *    benefit that'll be.
 */

#include "shader/uniform_buffer.h"

/**
 * UniformStruct implementation.
 */

/** @return             List of globally declared uniform structures. */
static UniformStruct::StructList &uniformStructList() {
    /* Lazily initialized to avoid global constructor order problems. */
    static UniformStruct::StructList uniformStructs;
    return uniformStructs;
}

/** Constructor for a statically declared uniform structure.
 * @param inName        Name of the structure.
 * @param inInstance    Instance name to use when declaring in shaders.
 * @param inSlot        Uniform slot to bind to when used in shaders.
 * @param inSize        Size of the structure.
 * @param init          Function to populate the member list. */
UniformStruct::UniformStruct(
    const char *inName,
    const char *inInstance,
    unsigned inSlot,
    size_t inSize,
    InitFunc init)
:
    name(inName),
    instanceName(inInstance),
    slot(inSlot),
    size(inSize)
{
    init(this);

    /* Register the structure. */
    uniformStructList().push_back(this);
}

/** Find a member in the structure by name.
 * @param name          Name of the member to find.
 * @return              Pointer to member if found, null if not. */
const UniformStructMember *UniformStruct::lookupMember(const char *name) const {
    for (const UniformStructMember &member : this->members) {
        if (strcmp(member.name, name) == 0)
            return &member;
    }

    return nullptr;
}

/** Add a new member to a dynamic uniform structure.
 * @param name          Name of the member to add.
 * @param type          Type of the member.
 * @return              Pointer to added member. */
const UniformStructMember *UniformStruct::addMember(const char *name, ShaderParameter::Type type) {
    this->members.emplace_back();

    UniformStructMember *member = &this->members.back();
    member->name = name;
    member->type = type;
    member->offset = Math::roundUp(this->size, ShaderParameter::alignment(type));

    this->size = member->offset + ShaderParameter::size(type);
    return member;
}

/** Add a new member to a static uniform structure.
 * @param name          Name of the member to add.
 * @param type          Type of the member.
 * @param offset        Offset of the member.
 * @return              Pointer to added member. */
const UniformStructMember *UniformStruct::addMember(const char *name, ShaderParameter::Type type, size_t offset) {
    this->members.emplace_back();

    UniformStructMember *member = &this->members.back();
    member->name = name;
    member->type = type;
    member->offset = offset;
    return member;
}

/** Get a list of globally declared uniform structures.
 * @return              List of globally declared uniform structures. */
const UniformStruct::StructList &UniformStruct::structList() {
    return uniformStructList();
}

/** Look up a globally declared uniform structure by name.
 * @param name          Name of structure to look up.
 * @return              Pointer to structure if found, null if not. */
const UniformStruct *UniformStruct::lookup(const std::string &name) {
    /* TODO: Do we need to add a separate lookup map or anything? Not perf
     * critical, it's only used at shader load time. */
    for (const UniformStruct *uniformStruct : uniformStructList()) {
        if (name == uniformStruct->name)
            return uniformStruct;
    }

    return nullptr;
}

/**
 * UniformBuffer implementation.
 */

/** Create the buffer, with zeroed content.
 * @param ustruct       Uniform structure type.
 * @param usage         GPU usage hint for the buffer. */
UniformBufferBase::UniformBufferBase(const UniformStruct &ustruct, GPUBuffer::Usage usage) :
    m_uniformStruct(ustruct),
    m_dirty(true)
{
    m_gpuBuffer = g_gpuManager->createBuffer(GPUBuffer::kUniformBuffer, usage, m_uniformStruct.size);
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
 * @return              Pointer to GPU buffer.
 */
GPUBuffer *UniformBufferBase::gpu() const {
    if (m_dirty) {
        m_gpuBuffer->write(0, m_uniformStruct.size, m_shadowBuffer);
        m_dirty = false;
    }

    return m_gpuBuffer;
}

/** Get the value of a member.
 * @param member        Details of the member to get.
 * @param buf           Where to store member value. */
void UniformBufferBase::readMember(const UniformStructMember *member, void *buf) const {
    memcpy(buf, m_shadowBuffer + member->offset, ShaderParameter::size(member->type));
}

/** Get the value of member.
 * @param name          Name of the member to get.
 * @param type          Expected type of the member (checked with an assertion).
 * @param buf           Where to store member value. */
void UniformBufferBase::readMember(const char *name, ShaderParameter::Type type, void *buf) const {
    const UniformStructMember *member = m_uniformStruct.lookupMember(name);

    checkMsg(
        member,
        "Member '%s' in uniform struct '%s' not found", name, m_uniformStruct.name);
    checkMsg(
        member->type == type,
        "Member '%s' in uniform struct '%s' incorrect type", name, m_uniformStruct.name);

    readMember(member, buf);
}

/** Set the value of a member.
 * @param member        Details of the member to set.
 * @param buf           Buffer containing new member value. */
void UniformBufferBase::writeMember(const UniformStructMember *member, const void *buf) const {
    m_dirty = true;
    memcpy(m_shadowBuffer + member->offset, buf, ShaderParameter::size(member->type));
}

/** Set the value of member.
 * @param name          Name of the member to set.
 * @param type          Expected type of the member (checked with an assertion).
 * @param buf           Buffer containing new member value. */
void UniformBufferBase::writeMember(const char *name, ShaderParameter::Type type, const void *buf) {
    const UniformStructMember *member = m_uniformStruct.lookupMember(name);

    checkMsg(
        member,
        "Member '%s' in uniform struct '%s' not found", name, m_uniformStruct.name);
    checkMsg(
        member->type == type,
        "Member '%s' in uniform struct '%s' incorrect type", name, m_uniformStruct.name);

    writeMember(member, buf);
}
