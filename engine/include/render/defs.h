/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Rendering definitions.
 */

#pragma once

#include "core/core.h"

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
 * Shader parameter type information.
 */

/** Enumeration of shader parameter types. */
enum class ShaderParameterType {
	/** Types that can be used in uniform structures. */
	kInt,				/**< Signed 32-bit integer. */
	kUnsignedInt,			/**< Unsigned 32-bit integer. */
	kFloat,				/**< Single-precision floating point. */
	kVec2,				/**< 2 component floating point vector. */
	kVec3,				/**< 3 component floating point vector. */
	kVec4,				/**< 4 component floating point vector. */
	kMat2,				/**< 2x2 floating point matrix. */
	kMat3,				/**< 3x3 floating point matrix. */
	kMat4,				/**< 4x4 floating point matrix. */

	/** Other types that cannot be used in uniform structures. */
	kTexture,			/**< Texture. */
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
	static constexpr ShaderParameterType kType = ShaderParameterType::kInt;
	static constexpr size_t kAlignment = 4;
};

template <>
struct ShaderParameterTypeTraits<uint32_t> {
	static constexpr ShaderParameterType kType = ShaderParameterType::kUnsignedInt;
	static constexpr size_t kAlignment = 4;
};

template <>
struct ShaderParameterTypeTraits<float> {
	static constexpr ShaderParameterType kType = ShaderParameterType::kFloat;
	static constexpr size_t kAlignment = 4;
};

template <>
struct ShaderParameterTypeTraits<glm::vec2> {
	static constexpr ShaderParameterType kType = ShaderParameterType::kVec2;
	static constexpr size_t kAlignment = 8;
};

template <>
struct ShaderParameterTypeTraits<glm::vec3> {
	static constexpr ShaderParameterType kType = ShaderParameterType::kVec3;
	static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::vec4> {
	static constexpr ShaderParameterType kType = ShaderParameterType::kVec4;
	static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::mat2> {
	static constexpr ShaderParameterType kType = ShaderParameterType::kMat2;
	static constexpr size_t kAlignment = 8;
};

template <>
struct ShaderParameterTypeTraits<glm::mat3> {
	static constexpr ShaderParameterType kType = ShaderParameterType::kMat3;
	static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::mat4> {
	static constexpr ShaderParameterType kType = ShaderParameterType::kMat4;
	static constexpr size_t kAlignment = 16;
};
