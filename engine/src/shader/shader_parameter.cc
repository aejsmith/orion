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
 * @brief               Shader parameter type information.
 */

#include "shader/shader_parameter.h"

/** Get the storage size for a shader parameter type.
 * @param type          Type to get size of. Only valid for basic types.
 * @return              Size of the type. */
size_t ShaderParameter::size(Type type) {
    switch (type) {
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
        case kIntVec2Type:
            return sizeof(glm::ivec2);
        case kIntVec3Type:
            return sizeof(glm::ivec3);
        case kIntVec4Type:
            return sizeof(glm::ivec4);
        default:
            unreachable();
    }
}

/** Get the uniform buffer alignment for a shader parameter type.
 * @param type          Type to get alignment of. Only valid for basic types.
 * @return              Alignment of the type. */
size_t ShaderParameter::alignment(Type type) {
    switch (type) {
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
        case kIntVec2Type:
            return ShaderParameterTypeTraits<glm::ivec2>::kAlignment;
        case kIntVec3Type:
            return ShaderParameterTypeTraits<glm::ivec3>::kAlignment;
        case kIntVec4Type:
            return ShaderParameterTypeTraits<glm::ivec4>::kAlignment;
        default:
            unreachable();
    }
}

/** Get the GLSL type string for a shader parameter type.
 * @param type          Type to get string for. Only valid for basic types.
 * @return              GLSL type string corresponding to the type. */
const char *ShaderParameter::glslType(Type type) {
    switch (type) {
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
        case kIntVec2Type:
            return "ivec2";
        case kIntVec3Type:
            return "ivec3";
        case kIntVec4Type:
            return "ivec4";
        default:
            unreachable();
    }
}
