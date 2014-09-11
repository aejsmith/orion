/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Uniform buffer helper classes.
 */

#pragma once

#include "gpu/gpu.h"

/**
 * Class maintaining a dynamically updated uniform buffer.
 *
 * This is a helper class for an object which contains a dynamically updated
 * uniform buffer. It only updates the uniform buffer when it is actually
 * needed. Whenever the data in the uniform buffer becomes outdated, the derived
 * class should call invalidate(), and the next time get() is called the buffer
 * will be updated. The entire previous buffer content is thrown away as
 * performing a partial update can cause a GPU synchronization.
 *
 * @tparam Uniforms	Type of the uniform structure.
 */
template <typename Uniforms>
class DynamicUniformBuffer {
public:
	DynamicUniformBuffer() : m_valid(false) {}

	/** Get uniform buffer for the object, updating if necessary.
	 * @param updater	Function to update the buffer if required.
	 *			The entire buffer is invalidated when this is
	 *			called so must be recreated from scratch.
	 * @return		Pointer to uniform buffer for the object. */
	template<typename Func> GPUBufferPtr get(Func func) {
		if(!m_valid) {
			/* Create the uniform buffer if it does not exist. */
			if(!m_buffer) {
				m_buffer = g_gpu->createBuffer(
					GPUBuffer::kUniformBuffer,
					GPUBuffer::kDynamicDrawUsage,
					sizeof(Uniforms));
			}

			GPUBufferMapper<Uniforms> uniforms(
				m_buffer,
				GPUBuffer::kMapInvalidate,
				GPUBuffer::kWriteAccess);

			func(uniforms);
			m_valid = true;
		}

		return m_buffer;
	}

	/** Mark the uniforms as invalid. */
	void invalidate() { m_valid = false; }
private:
	GPUBufferPtr m_buffer;		/**< Uniform buffer. */
	bool m_valid;			/**< Whether the buffer contents are valid. */
};

/**
 * Uniform structure member type information.
 */

/** Enumeration of uniform types. */
enum class UniformType {
	kIntType,			/**< Signed 32-bit integer. */
	kUnsignedIntType,		/**< Unsigned 32-bit integer. */
	kFloatType,			/**< Single-precision floating point. */
};

/** Structure providing information about a uniform type. */
template <typename T>
struct UniformTypeInfo;

template <>
struct UniformTypeInfo<int32_t> {
	static const UniformType kBaseType = UniformType::kIntType;
	static const size_t kRows = 1;
	static const size_t kColumns = 1;
	static const size_t kAlignment = 4;
};

template <>
struct UniformTypeInfo<uint32_t> {
	static const UniformType kBaseType = UniformType::kUnsignedIntType;
	static const size_t kRows = 1;
	static const size_t kColumns = 1;
	static const size_t kAlignment = 4;
};

template <>
struct UniformTypeInfo<float> {
	static const UniformType kBaseType = UniformType::kFloatType;
	static const size_t kRows = 1;
	static const size_t kColumns = 1;
	static const size_t kAlignment = 4;
};

template <typename T>
struct UniformTypeInfo<glm::detail::tvec2<T, glm::highp>> {
	static const UniformType kBaseType = UniformTypeInfo<T>::kBaseType;
	static const size_t kRows = 1;
	static const size_t kColumns = 2;
	static const size_t kAlignment = 2 * UniformTypeInfo<T>::kAlignment;
};

template <typename T>
struct UniformTypeInfo<glm::detail::tvec3<T, glm::highp>> {
	static const UniformType kBaseType = UniformTypeInfo<T>::kBaseType;
	static const size_t kRows = 1;
	static const size_t kColumns = 3;
	static const size_t kAlignment = 4 * UniformTypeInfo<T>::kAlignment;
};

template <typename T>
struct UniformTypeInfo<glm::detail::tvec4<T, glm::highp>> {
	static const UniformType kBaseType = UniformTypeInfo<T>::kBaseType;
	static const size_t kRows = 1;
	static const size_t kColumns = 4;
	static const size_t kAlignment = 4 * UniformTypeInfo<T>::kAlignment;
};

template <typename T>
struct UniformTypeInfo<glm::detail::tmat2x2<T, glm::highp>> {
	static const UniformType kBaseType = UniformTypeInfo<T>::kBaseType;
	static const size_t kRows = 2;
	static const size_t kColumns = 2;
	static const size_t kAlignment = UniformTypeInfo<glm::detail::tvec2<T, glm::highp>>::kAlignment;
};

template <typename T>
struct UniformTypeInfo<glm::detail::tmat3x3<T, glm::highp>> {
	static const UniformType kBaseType = UniformTypeInfo<T>::kBaseType;
	static const size_t kRows = 3;
	static const size_t kColumns = 3;
	static const size_t kAlignment = UniformTypeInfo<glm::detail::tvec3<T, glm::highp>>::kAlignment;
};

template <typename T>
struct UniformTypeInfo<glm::detail::tmat4x4<T, glm::highp>> {
	static const UniformType kBaseType = UniformTypeInfo<T>::kBaseType;
	static const size_t kRows = 4;
	static const size_t kColumns = 4;
	static const size_t kAlignment = UniformTypeInfo<glm::detail::tvec4<T, glm::highp>>::kAlignment;
};

/**
 * Uniform structure metadata.
 */

/** Information about a uniform structure. */
struct UniformStruct {
	/** Information about a uniform structure member. */
	struct Member {
		const char *name;	/**< Name of the member. */
		UniformType baseType;	/**< Base type. */
		size_t rows;		/**< Rows (for matrix types). */
		size_t columns;		/**< Columns (for matrix/vector types). */
		size_t offset;		/**< Offset of the member. */
	};

	/** Type of the member variable array. */
	typedef std::vector<Member> MemberArray;

	/** Type of the member array initialization function. */
	typedef void (*InitMembersFunc)(MemberArray &);
public:
	const char *name;		/**< Name of the structure. */
	size_t size;			/**< Size of the structure. */
	MemberArray members;		/**< Members of the structure. */
public:
	UniformStruct(const char *inName, size_t inSize, InitMembersFunc initFunc) :
		name(inName),
		size(inSize)
	{
		initFunc(this->members);
	}
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
				UniformTypeInfo<typeName>::kBaseType, \
				UniformTypeInfo<typeName>::kRows, \
				UniformTypeInfo<typeName>::kColumns, \
				offsetof(UniformStructType, memberName), \
			}; \
			static_assert( \
				(offsetof(UniformStructType, memberName) & (UniformTypeInfo<typeName>::kAlignment - 1)) == 0, \
				"Uniform buffer member " #memberName " is misaligned"); \
			_members.push_back(_memberInfo); \
			_initMembers_##memberName(_members); \
		} \
		\
		alignas(UniformTypeInfo<typeName>::kAlignment) typeName memberName; \
		\
		static void _initMembers_##memberName(UniformStruct::MemberArray &_members) {

/** End a uniform buffer structure declaration. */
#define UNIFORM_STRUCT_END \
		} \
	}

/** Define uniform structure metadata.
 * @param structName	Name of the structure. */
#define IMPLEMENT_UNIFORM_STRUCT(structName) \
	const UniformStruct structName::kUniformStruct(#structName, sizeof(structName), structName::_initMembers)
