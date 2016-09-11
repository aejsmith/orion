/*
 * Copyright (C) 2015-2016 Alex Smith
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
 */

#pragma once

#include "gpu/gpu_manager.h"

#include "shader/shader_parameter.h"

#include <list>

/**
 * Uniform structure metadata.
 */

/** Information about a uniform structure member.
 * @note                Declared globally rather than as a nested class within
 *                      UniformStruct to avoid a cyclic dependency between
 *                      this header and shader_parameter.h. */
struct UniformStructMember {
    const char *name;                   /**< Name of the member. */
    ShaderParameter::Type type;         /**< Member type. */
    size_t offset;                      /**< Offset of the member. */
};

/** Information about a uniform structure. */
class UniformStruct {
public:
    /** Type of the global uniform structure list. */
    using StructList = std::list<UniformStruct *>;

    /** Type of the member variable list. */
    using MemberList = std::list<UniformStructMember>;

    /** Type of the member initialization function. */
    using InitFunc = void (*)(UniformStruct *);

    /** Constructor for a dynamically-built uniform structure.
     * @param inName        Name of the structure.
     * @param inInstance    Instance name to use when declaring in shaders.
     * @param inSet         Resource set to bind to in shaders. The slot used is
     *                      the standard ResourceSlots::kUniforms. */
    UniformStruct(const char *inName, const char *inInstance, unsigned inSet) :
        name(inName),
        instanceName(inInstance),
        set(inSet),
        m_size(0)
    {}

    UniformStruct(const char *inName, const char *inInstance, unsigned inSet, size_t size, InitFunc init);

    const char *name;                   /**< Name of the structure. */
    const char *instanceName;           /**< Instance name to use when declaring in shaders. */
    unsigned set;                       /**< Resource set to bind to in shaders. */

    const UniformStructMember *lookupMember(const char *name) const;

    const UniformStructMember *addMember(const char *name, ShaderParameter::Type type);
    const UniformStructMember *addMember(const char *name, ShaderParameter::Type type, size_t offset);

    /** @return             Size of the structure. */
    size_t size() const { return m_size; }
    /** @return             List of members. */
    const MemberList &members() const { return m_members; }

    static const StructList &structList();
    static const UniformStruct *lookup(const std::string &name);
private:
    size_t m_size;                      /**< Size of the structure. */
    MemberList m_members;               /**< Members of the structure. */
};

/**
 * Uniform structure declaration.
 */

/** Begin a uniform structure declaration.
 * @param structName    Name of the structure. */
#define UNIFORM_STRUCT_BEGIN(structName) \
    struct structName { \
        using UniformStructType = structName; \
        static const UniformStruct kUniformStruct; \
        \
        static void _initMembers(UniformStruct *_struct) { \

/** Declare a uniform structure member.
 * @param typeName      Type of the member.
 * @param memberName    Name of the member. */
#define UNIFORM_STRUCT_MEMBER(typeName, memberName) \
            static_assert( \
                (offsetof(UniformStructType, memberName) \
                    & (ShaderParameterTypeTraits<typeName>::kAlignment - 1)) == 0, \
                "Uniform buffer member " #memberName " is misaligned"); \
            _struct->addMember( \
                #memberName, \
                ShaderParameterTypeTraits<typeName>::kType, \
                offsetof(UniformStructType, memberName)); \
            _initMembers_##memberName(_struct); \
        } \
        \
        alignas(ShaderParameterTypeTraits<typeName>::kAlignment) typeName memberName; \
        \
        static void _initMembers_##memberName(UniformStruct *_struct) {

/** End a uniform buffer structure declaration. */
#define UNIFORM_STRUCT_END \
        } \
    }

/** Define uniform structure metadata.
 * @param structName    Name of the structure.
 * @param instanceName  Instance name to use when declaring in shaders.
 * @param set           Resource set to bind to in shaders. The slot used is
 *                      the standard ResourceSlots::kUniforms. */
#define IMPLEMENT_UNIFORM_STRUCT(structName, instanceName, set) \
    const UniformStruct structName::kUniformStruct( \
        #structName, \
        instanceName, \
        set, \
        sizeof(structName), \
        structName::_initMembers);

/**
 * Uniform buffer helper classes.
 */

/**
 * Uniform buffer wrapper class.
 *
 * This class maintains a uniform buffer. It uses uniform structure type
 * information to be able to generically modify members. It also keeps a
 * CPU-side shadow buffer to make it possible to read members and perform
 * partial updates without causing GPU synchronizations.
 */
class UniformBufferBase {
public:
    UniformBufferBase(const UniformStruct &ustruct, GPUBuffer::Usage usage = GPUBuffer::kDynamicUsage);
    ~UniformBufferBase();

    /** @return             Uniform structure for this buffer. */
    const UniformStruct &uniformStruct() const { return m_uniformStruct; }
    /** @return             Backing GPU buffer. */
    GPUBuffer *gpu() const { return m_gpu; }

    void flush() const;

    /**
     * Member access.
     */

    void readMember(const UniformStructMember *member, void *buf) const;
    void readMember(const char *name, ShaderParameter::Type type, void *buf) const;
    void writeMember(const UniformStructMember *member, const void *buf) const;
    void writeMember(const char *name, ShaderParameter::Type type, const void *buf);

    /** Get a member from the buffer.
     * @tparam T            Type of the member.
     * @param name          Name of the member to get.
     * @return              Member value. */
    template <typename T> T readMember(const char *name) {
        T ret;
        readMember(name, ShaderParameterTypeTraits<T>::kType, std::addressof(ret));
        return ret;
    }

    /** Set a member in the buffer.
     * @tparam T            Type of the member.
     * @param name          Name of the member to set.
     * @param value         Value to set to. */
    template <typename T> void writeMember(const char *name, const T &value) {
        writeMember(name, ShaderParameterTypeTraits<T>::kType, std::addressof(value));
    }
protected:
    const UniformStruct &m_uniformStruct;   /**< Uniform structure for the buffer. */
    GPUBufferPtr m_gpu;                     /**< GPU buffer. */
    char *m_shadowBuffer;                   /**< CPU shadow buffer. */
    mutable bool m_dirty;                   /**< Whether the buffer is dirty. */
};

/**
 * Statically-typed uniform buffer.
 *
 * This class is a template version of UniformBufferBase which has a type
 * defined at compile time. In addition to UniformBufferBase, it adds methods
 * for direct access to the buffer contents.
 *
 * @tparam Uniforms Uniform structure.
 */
template <typename Uniforms>
class UniformBuffer : public UniformBufferBase {
public:
    /** Initialize the buffer.
     * @param usage         GPU buffer usage hint. */
    explicit UniformBuffer(GPUBuffer::Usage usage = GPUBuffer::kDynamicUsage) :
        UniformBufferBase(Uniforms::kUniformStruct, usage)
    {}

    /** Access the buffer for reading.
     * @return              Pointer to the buffer for reading. */
    const Uniforms *read() const {
        return reinterpret_cast<const Uniforms *>(m_shadowBuffer);
    }

    /**
     * Access the buffer for writing.
     *
     * Accesses the buffer contents for writing. This accesses the CPU shadow
     * buffer, and sets a flag to indicate that the buffer content is dirty.
     * Pending modifications will be flushed next time flush() is called. Note
     * that since the dirty flag is set only when this function is called, you
     * should not save the returned pointer across across a call to flush() as
     * writes may not be flushed.
     *
     * @return              Pointer to the buffer for writing.
     */
    Uniforms *write() {
        m_dirty = true;
        return reinterpret_cast<Uniforms *>(m_shadowBuffer);
    }
};
