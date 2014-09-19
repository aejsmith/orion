/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Shader parameter type information.
 */

#include "render/defs.h"

/** Get the storage size for a shader parameter type.
 * @param type		Type to get size of. Only valid for basic types.
 * @return		Size of the type. */
size_t ShaderParameter::size(Type type) {
	switch(type) {
	case kIntType:
		return sizeof(int32_t);
	case kUnsignedIntType:
		return sizeof(uint32_t);
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
		orionAbort("Invalid type passed to ShaderParameter::size");
	}
}

/** Get the uniform buffer alignment for a shader parameter type.
 * @param type		Type to get alignment of. Only valid for basic types.
 * @return		Alignment of the type. */
size_t ShaderParameter::alignment(Type type) {
	switch(type) {
	case kIntType:
		return ShaderParameterTypeTraits<int32_t>::kAlignment;
	case kUnsignedIntType:
		return ShaderParameterTypeTraits<uint32_t>::kAlignment;
	case kFloatType:
		return ShaderParameterTypeTraits<float>::kAlignment;
	case kVec2Type:
		return ShaderParameterTypeTraits<glm::vec2>::kAlignment;
	case kVec3Type:
		return ShaderParameterTypeTraits<glm::vec3>::kAlignment;
	case kVec4Type:
		return ShaderParameterTypeTraits<glm::vec4>::kAlignment;
	case kMat2Type:
		return ShaderParameterTypeTraits<glm::mat2>::kAlignment;
	case kMat3Type:
		return ShaderParameterTypeTraits<glm::mat3>::kAlignment;
	case kMat4Type:
		return ShaderParameterTypeTraits<glm::mat4>::kAlignment;
	default:
		orionAbort("Invalid type passed to ShaderParameter::alignment");
	}
}

/** Get the GLSL type string for a shader parameter type.
 * @param type		Type to get string for. Only valid for basic types.
 * @return		GLSL type string corresponding to the type. */
const char *ShaderParameter::glslType(Type type) {
	switch(type) {
	case kIntType:
		return "int";
	case kUnsignedIntType:
		return "uint";
	case kFloatType:
		return "float";
	case kVec2Type:
		return "vec2";
	case kVec3Type:
		return "vec3";
	case kVec4Type:
		return "vec4";
	case kMat2Type:
		return "mat2";
	case kMat3Type:
		return "mat3";
	case kMat4Type:
		return "mat4";
	default:
		orionAbort("Invalid type passed to ShaderParameter::glslType");
	}
}
