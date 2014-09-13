/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Uniform buffer classes.
 */

#pragma once

#include "gpu/gpu.h"

#include "render/defs.h"

/**
 * Uniform structure metadata.
 */

/** Information about a uniform structure. */
struct UniformStruct {
	/** Information about a uniform structure member. */
	struct Member {
		const char *name;		/**< Name of the member. */
		ShaderParameterType type;	/**< Member type. */
		size_t offset;			/**< Offset of the member. */
	};

	/** Type of the member variable array. */
	typedef std::vector<Member> MemberArray;

	/** Type of the member array initialization function. */
	typedef void (*InitMembersFunc)(MemberArray &);
public:
	const char *name;			/**< Name of the structure. */
	size_t size;				/**< Size of the structure. */
	MemberArray members;			/**< Members of the structure. */
public:
	UniformStruct(const char *inName, size_t inSize, InitMembersFunc initFunc) :
		name(inName),
		size(inSize)
	{
		initFunc(this->members);
	}

	const Member *lookupMember(const char *name) const;
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
		static void _initMembers(UniformStruct::MemberArray &_members) { \

/** Declare a uniform structure member.
 * @param typeName	Type of the member.
 * @param memberName	Name of the member. */
#define UNIFORM_STRUCT_MEMBER(typeName, memberName) \
			UniformStruct::Member _memberInfo = { \
				#memberName, \
				ShaderParameterTypeTraits<typeName>::kType, \
				offsetof(UniformStructType, memberName), \
			}; \
			static_assert( \
				(offsetof(UniformStructType, memberName) & (ShaderParameterTypeTraits<typeName>::kAlignment - 1)) == 0, \
				"Uniform buffer member " #memberName " is misaligned"); \
			_members.push_back(_memberInfo); \
			_initMembers_##memberName(_members); \
		} \
		\
		alignas(ShaderParameterTypeTraits<typeName>::kAlignment) typeName memberName; \
		\
		static void _initMembers_##memberName(UniformStruct::MemberArray &_members) {

/** End a uniform buffer structure declaration. */
#define UNIFORM_STRUCT_END \
		} \
	}

/** Define uniform structure metadata.
 * @param structName	Name of the structure. */
#define IMPLEMENT_UNIFORM_STRUCT(structName) \
	const UniformStruct structName::kUniformStruct(#structName, sizeof(structName), structName::_initMembers);

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

	void getMember(const UniformStruct::Member *member, void *value) const;
	void getMember(const char *name, ShaderParameterType type, void *value) const;
	void setMember(const UniformStruct::Member *member, const void *value) const;
	void setMember(const char *name, ShaderParameterType type, const void *value);

	/** Get a member from the buffer.
	 * @tparam T		Type of the member.
	 * @param name		Name of the member to get.
	 * @param value		Where to store member value. */
	template <typename T> void getMember(const char *name, T &value) {
		getMember(name, ShaderParameterTypeTraits<T>::kType, &value);
	}

	/** Set a member in the buffer.
	 * @tparam T		Type of the member.
	 * @param name		Name of the member to set.
	 * @param value		Value to set to. */
	template <typename T> void setMember(const char *name, const T &value) {
		setMember(name, ShaderParameterTypeTraits<T>::kType, &value);
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
 * @tparam Struct	Uniform structure.
 */
template <typename Struct>
class UniformBuffer : public UniformBufferBase {
public:
	/** Initialize the buffer.
	 * @param usage		GPU buffer usage hint. */
	explicit UniformBuffer(GPUBuffer::Usage usage = GPUBuffer::kDynamicDrawUsage) :
		UniformBufferBase(Struct::kUniformStruct, usage)
	{}

	/** Access the buffer for reading.
	 * @return		Pointer to the buffer for reading. */
	const Struct *read() const {
		return reinterpret_cast<const Struct *>(m_shadowBuffer);
	}

	/**
	 * Access the buffer for writing.
	 *
	 * Accesses the buffer contents for writing. This accesses the CPU
	 * shadow buffer, and sets a flag to indicate that the buffer content
	 * is dirty. Pending modifications will be flushed next time the GPU
	 * buffer is requested. Note that since the dirty flag is set only when
	 * this function is called, you should not save the returned pointer,
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
	Struct *write() {
		m_dirty = true;
		return reinterpret_cast<Struct *>(m_shadowBuffer);
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
	Struct *operator ->() { return write(); }
};
