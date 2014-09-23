/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Uniform buffer classes.
 */

#pragma once

#include "gpu/gpu.h"

#include "render/shader_parameter.h"
#include "render/slots.h"

#include <list>

/**
 * Uniform structure metadata.
 */

/** Information about a uniform structure member. */
struct UniformStructMember {
	const char *name;		/**< Name of the member. */
	ShaderParameter::Type type;	/**< Member type. */
	size_t offset;			/**< Offset of the member. */
};

/** Information about a uniform structure. */
struct UniformStruct {
	/** Type of the global uniform structure list. */
	typedef std::list<UniformStruct *> StructList;

	/** Type of the member variable list. */
	typedef std::list<UniformStructMember> MemberList;

	/** Type of the member initialization function. */
	typedef void (*InitMembersFunc)(UniformStruct *);
public:
	const char *name;			/**< Name of the structure. */
	const char *instanceName;		/**< Instance name to use when declaring in shaders. */
	unsigned slot;				/**< Uniform slot to bind to when used in shaders. */
	size_t size;				/**< Size of the structure. */
	MemberList members;			/**< Members of the structure. */
public:
	/** Constructor for a dynamically built uniform structure. */
	UniformStruct(const char *inName, const char *inInstance, unsigned inSlot) :
		name(inName),
		instanceName(inInstance),
		slot(inSlot),
		size(0)
	{}

	UniformStruct(const char *inName, const char *inInstance, unsigned inSlot, size_t inSize, InitMembersFunc initFunc);

	const UniformStructMember *lookupMember(const char *name) const;

	const UniformStructMember *addMember(const char *name, ShaderParameter::Type type);
	const UniformStructMember *addMember(const char *name, ShaderParameter::Type type, size_t offset);

	static const StructList &structList();
	static const UniformStruct *lookup(const std::string &name);
};

/**
 * Uniform structure declaration.
 */

/** Begin a uniform structure declaration.
 * @param structName	Name of the structure. */
#define UNIFORM_STRUCT_BEGIN(structName) \
	struct structName { \
		typedef structName UniformStructType; \
		static const UniformStruct kUniformStruct; \
		\
		static void _initMembers(UniformStruct *_struct) { \

/** Declare a uniform structure member.
 * @param typeName	Type of the member.
 * @param memberName	Name of the member. */
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
 * @param structName	Name of the structure.
 * @param instanceName	Instance name to use when declaring in shaders.
 * @param slot		Uniform slot to bind to when used in shaders. */
#define IMPLEMENT_UNIFORM_STRUCT(structName, instanceName, slot) \
	const UniformStruct structName::kUniformStruct( \
		#structName, \
		instanceName, \
		slot, \
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
	UniformBufferBase(const UniformStruct &ustruct, GPUBuffer::Usage usage = GPUBuffer::kDynamicDrawUsage);
	~UniformBufferBase();

	/** @return		Uniform structure for this buffer. */
	const UniformStruct &uniformStruct() const { return m_uniformStruct; }

	GPUBufferPtr gpu() const;

	/**
	 * Member access.
	 */

	void readMember(const UniformStructMember *member, void *buf) const;
	void readMember(const char *name, ShaderParameter::Type type, void *buf) const;
	void writeMember(const UniformStructMember *member, const void *buf) const;
	void writeMember(const char *name, ShaderParameter::Type type, const void *buf);

	/** Get a member from the buffer.
	 * @tparam T		Type of the member.
	 * @param name		Name of the member to get.
	 * @return		Member value. */
	template <typename T> T readMember(const char *name) {
		T ret;
		readMember(name, ShaderParameterTypeTraits<T>::kType, std::addressof(ret));
		return ret;
	}

	/** Set a member in the buffer.
	 * @tparam T		Type of the member.
	 * @param name		Name of the member to set.
	 * @param value		Value to set to. */
	template <typename T> void writeMember(const char *name, const T &value) {
		writeMember(name, ShaderParameterTypeTraits<T>::kType, std::addressof(value));
	}
protected:
	const UniformStruct &m_uniformStruct;	/**< Uniform structure for the buffer. */
	GPUBufferPtr m_gpuBuffer;		/**< GPU buffer. */
	char *m_shadowBuffer;			/**< CPU shadow buffer. */
	mutable bool m_dirty;			/**< Whether the buffer is dirty. */
};

/**
 * Statically-typed uniform buffer.
 *
 * This class is a template version of UniformBufferBase which has a type
 * defined at compile time. In addition to UniformBufferBase, it adds methods
 * for direct access to the buffer contents.
 *
 * @tparam Uniforms	Uniform structure.
 */
template <typename Uniforms>
class UniformBuffer : public UniformBufferBase {
public:
	/** Initialize the buffer.
	 * @param usage		GPU buffer usage hint. */
	explicit UniformBuffer(GPUBuffer::Usage usage = GPUBuffer::kDynamicDrawUsage) :
		UniformBufferBase(Uniforms::kUniformStruct, usage)
	{}

	/** Access the buffer for reading.
	 * @return		Pointer to the buffer for reading. */
	const Uniforms *read() const {
		return reinterpret_cast<const Uniforms *>(m_shadowBuffer);
	}

	/**
	 * Access the buffer for writing.
	 *
	 * Accesses the buffer contents for writing. This accesses the CPU
	 * shadow buffer, and sets a flag to indicate that the buffer content
	 * is dirty. Pending modifications will be flushed next time the GPU
	 * buffer is requested. Note that since the dirty flag is set only when
	 * this function is called, you should not save the returned pointer
	 * across a call to gpu() as writes may not be flushed. For example:
	 *
	 *  MyUniforms *uniforms = m_uniforms.write();
	 *  uniforms->foo = 42;
	 *  g_gpu->bind_uniform_buffer(m_uniforms.gpu());
	 *  uniforms->bar = 1234;
	 *
	 * After the above sequence, the final write may not be flushed by the
	 * next call to gpu() unless something else calls write() inbetween.
	 *
	 * @return		Pointer to the buffer for writing.
	 */
	Uniforms *write() {
		m_dirty = true;
		return reinterpret_cast<Uniforms *>(m_shadowBuffer);
	}

	/**
	 * Dereference operator.
	 *
	 * Since in most cases when accessing a uniform buffer we want to write
	 * to it (the content of a uniform buffer is usually a mirror of some
	 * engine-side data), we provide a dereference operator that is
	 * equivalent to write() to make writing code look a bit neater.
	 *
	 * @return		Pointer to the buffer for writing.
	 */
	Uniforms *operator ->() { return write(); }
};
