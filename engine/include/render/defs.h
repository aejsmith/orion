/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Rendering definitions.
 */

#pragma once

#include "core/refcounted.h"

class Texture2D;
class TextureBase;

struct UniformStructMember;

/**
 * Standard uniform buffer binding point indices.
 *
 * We define a set of uniform buffer binding point indices for standard things
 * like per-entity/light/view uniforms. These will be bound automatically by
 * the renderer if they are used by a shader. Shader-specific uniform buffers
 * can be bound to indices in the custom range, and should be bound by shader
 * C++ code.
 */
namespace UniformSlots {
	enum {
		/** Uniforms for the entity currently being rendered. */
		kEntityUniforms = 0,
		/** Uniforms for the light for the current pass. */
		kLightUniforms = 1,
		/** Uniforms for the view the scene is being rendered from. */
		kViewUniforms = 2,

		/** Custom (shader-specific) uniform block range. */
		kCustomUniformsStart = 8,
		kCustomUniformsEnd = 15,
	};
}

/**
 * Standard texture binding point indices.
 *
 * Similarly to UniformSlots, we define a set of standard texture binding points
 * for use by shaders. Some of these refer to textures provided by the renderer,
 * e.g. G-Buffer textures, and are bound automatically. Shader texture
 * parameters are specified with an index and are automatically bound to the
 * textures specified in their materials.
 */
namespace TextureSlots {
	enum {
		/** Diffuse material texture. */
		kDiffuseTexture = 0,
		/** Normal map texture. */
		kNormalTexture = 1,

		/** Custom (shader-specific) texture range. */
		kCustomTextureStart = 8,
		kCustomTextureEnd = 15,
	};
}

/**
 * Shader parameter information.
 */

/** Details of a shader parameter. */
struct ShaderParameter {
	/** Enumeration of parameter types. */
	enum Type {
		/** Basic types. */
		kIntType,		/**< Signed 32-bit integer. */
		kUnsignedIntType,	/**< Unsigned 32-bit integer. */
		kFloatType,		/**< Single-precision floating point. */
		kVec2Type,		/**< 2 component floating point vector. */
		kVec3Type,		/**< 3 component floating point vector. */
		kVec4Type,		/**< 4 component floating point vector. */
		kMat2Type,		/**< 2x2 floating point matrix. */
		kMat3Type,		/**< 3x3 floating point matrix. */
		kMat4Type,		/**< 4x4 floating point matrix. */

		/** Special types (cannot be used in uniform structures). */
		kTextureType,		/**< Texture. */
	};

	/** Parameter binding type. */
	enum Binding {
		kNoBinding,		/**< No automatic binding (extra parameter). */
		kUniformBinding,	/**< Bind to a uniform struct member. */
		kTextureBinding,	/**< Bind to a texture slot. */
	};
public:
	const char *name;		/**< Parameter name. */
	Type type;			/**< Parameter type. */
	Binding binding;		/**< Binding type. */
	unsigned index;			/**< Index into material parameter tables. */

	/** Binding information. */
	union {
		/** Uniform struct member (kUniformParameterBinding). */
		const UniformStructMember *uniformMember;
		/** Texture slot (kTextureParameterBinding). */
		unsigned textureSlot;
	};
public:
	/** @return		Storage size of the parameter. */
	size_t size() const { return size(this->type); }

	/** Get the storage size for a shader parameter type.
	 * @param type		Type to get size of. Only valid for basic types.
	 * @return		Size of the type. */
	static size_t size(Type type) {
		switch(type) {
		case kIntType:
			return sizeof(int);
		case kUnsignedIntType:
			return sizeof(unsigned int);
		case kFloatType:
			return sizeof(float);
		case kVec2Type:
			return sizeof(glm::vec2);
		case kVec3Type:
			return sizeof(glm::vec3);
		case kVec4Type:
			return sizeof(glm::vec4);
		case kMat2Type:
			return sizeof(glm::mat2);
		case kMat3Type:
			return sizeof(glm::mat3);
		case kMat4Type:
			return sizeof(glm::mat4);
		default:
			/* Textures require special handling. */
			return 0;
		}
	}
};

/**
 * Structure providing information about a shader parameter type.
 *
 * This provides compile time information about shader parameter types. It
 * provides 2 constant members, kType which gives the type enumeration, and
 * kAlignment which gives the member alignment for uniform buffers. If a type
 * information structure does not include a kAlignment field, it cannot be used
 * in a uniform buffer.
 */
template <typename T>
struct ShaderParameterTypeTraits;

template <>
struct ShaderParameterTypeTraits<int32_t> {
	static constexpr ShaderParameter::Type kType = ShaderParameter::kIntType;
	static constexpr size_t kAlignment = 4;
};

template <>
struct ShaderParameterTypeTraits<uint32_t> {
	static constexpr ShaderParameter::Type kType = ShaderParameter::kUnsignedIntType;
	static constexpr size_t kAlignment = 4;
};

template <>
struct ShaderParameterTypeTraits<float> {
	static constexpr ShaderParameter::Type kType = ShaderParameter::kFloatType;
	static constexpr size_t kAlignment = 4;
};

template <>
struct ShaderParameterTypeTraits<glm::vec2> {
	static constexpr ShaderParameter::Type kType = ShaderParameter::kVec2Type;
	static constexpr size_t kAlignment = 8;
};

template <>
struct ShaderParameterTypeTraits<glm::vec3> {
	static constexpr ShaderParameter::Type kType = ShaderParameter::kVec3Type;
	static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::vec4> {
	static constexpr ShaderParameter::Type kType = ShaderParameter::kVec4Type;
	static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::mat2> {
	static constexpr ShaderParameter::Type kType = ShaderParameter::kMat2Type;
	static constexpr size_t kAlignment = 8;
};

template <>
struct ShaderParameterTypeTraits<glm::mat3> {
	static constexpr ShaderParameter::Type kType = ShaderParameter::kMat3Type;
	static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::mat4> {
	static constexpr ShaderParameter::Type kType = ShaderParameter::kMat4Type;
	static constexpr size_t kAlignment = 16;
};

/* Note: Texture2D is specifically left unimplemented here because at the moment
 * we don't type parameters to a specific texture type, just generic types.
 * Implementing Texture2D here makes Material::getValue unsafe. */
template <>
struct ShaderParameterTypeTraits<ReferencePtr<TextureBase>> {
	static constexpr ShaderParameter::Type kType = ShaderParameter::kTextureType;
};
