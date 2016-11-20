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

#include "render_core/shader_parameter.h"

/** Get the storage size for a shader parameter type.
 * @param type          Type to get size of. Only valid for basic types.
 * @return              Size of the type. */
size_t ShaderParameter::size(Type type) {
    switch (type) {
        case Type::kInt:
            return sizeof(int32_t);
        case Type::kUnsignedInt:
            return sizeof(uint32_t);
        case Type::kFloat:
            return sizeof(float);
        case Type::kVec2:
            return sizeof(glm::vec2);
        case Type::kVec3:
            return sizeof(glm::vec3);
        case Type::kVec4:
            return sizeof(glm::vec4);
        case Type::kMat2:
            return sizeof(glm::mat2);
        case Type::kMat3:
            return sizeof(glm::mat3);
        case Type::kMat4:
            return sizeof(glm::mat4);
        case Type::kIntVec2:
            return sizeof(glm::ivec2);
        case Type::kIntVec3:
            return sizeof(glm::ivec3);
        case Type::kIntVec4:
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
        case Type::kInt:
            return ShaderParameterTypeTraits<int32_t>::kAlignment;
        case Type::kUnsignedInt:
            return ShaderParameterTypeTraits<uint32_t>::kAlignment;
        case Type::kFloat:
            return ShaderParameterTypeTraits<float>::kAlignment;
        case Type::kVec2:
            return ShaderParameterTypeTraits<glm::vec2>::kAlignment;
        case Type::kVec3:
            return ShaderParameterTypeTraits<glm::vec3>::kAlignment;
        case Type::kVec4:
            return ShaderParameterTypeTraits<glm::vec4>::kAlignment;
        case Type::kMat2:
            return ShaderParameterTypeTraits<glm::mat2>::kAlignment;
        case Type::kMat3:
            return ShaderParameterTypeTraits<glm::mat3>::kAlignment;
        case Type::kMat4:
            return ShaderParameterTypeTraits<glm::mat4>::kAlignment;
        case Type::kIntVec2:
            return ShaderParameterTypeTraits<glm::ivec2>::kAlignment;
        case Type::kIntVec3:
            return ShaderParameterTypeTraits<glm::ivec3>::kAlignment;
        case Type::kIntVec4:
            return ShaderParameterTypeTraits<glm::ivec4>::kAlignment;
        default:
            unreachable();
    }
}

/** Get the GLSL type string for a shader parameter type.
 * @param type          Type to get string for.
 * @return              GLSL type string corresponding to the type. */
const char *ShaderParameter::glslType(Type type) {
    switch (type) {
        case Type::kInt:
            return "int";
        case Type::kUnsignedInt:
            return "uint";
        case Type::kFloat:
            return "float";
        case Type::kVec2:
            return "vec2";
        case Type::kVec3:
            return "vec3";
        case Type::kVec4:
            return "vec4";
        case Type::kMat2:
            return "mat2";
        case Type::kMat3:
            return "mat3";
        case Type::kMat4:
            return "mat4";
        case Type::kIntVec2:
            return "ivec2";
        case Type::kIntVec3:
            return "ivec3";
        case Type::kIntVec4:
            return "ivec4";
        case Type::kTexture2D:
            return "sampler2D";
        case Type::kTextureCube:
            return "samplerCube";
        default:
            unreachable();
    }
}
