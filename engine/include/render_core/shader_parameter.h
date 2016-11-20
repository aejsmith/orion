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
 * @brief               Shader parameter definitions.
 */

#pragma once

#include "engine/object.h"

class Texture2D;
class TextureCube;

struct UniformStructMember;

/** Details of a shader parameter. */
struct ShaderParameter {
    /** Enumeration of parameter types. */
    enum class ENUM() Type {
        /** Basic types. */
        kInt,                       /**< Signed 32-bit integer. */
        kUnsignedInt,               /**< Unsigned 32-bit integer. */
        kFloat,                     /**< Single-precision floating point. */
        kVec2,                      /**< 2 component floating point vector. */
        kVec3,                      /**< 3 component floating point vector. */
        kVec4,                      /**< 4 component floating point vector. */
        kMat2,                      /**< 2x2 floating point matrix. */
        kMat3,                      /**< 3x3 floating point matrix. */
        kMat4,                      /**< 4x4 floating point matrix. */
        kIntVec2,                   /**< 2 component integer vector. */
        kIntVec3,                   /**< 3 component integer vector. */
        kIntVec4,                   /**< 4 component integer vector. */

        /** Special types (cannot be used in uniform structures). */
        kTexture2D,                 /**< 2D texture. */
        kTextureCube,               /**< Cube texture. */
    };

    Type type;                      /**< Parameter type. */

    union {
        /** For uniform parameters, the struct member for the parameter. */
        const UniformStructMember *uniformMember;
        /** For other parameters, the resource slot to bind to. */
        unsigned resourceSlot;
    };

    /** @return             Storage size of the parameter. */
    size_t size() const { return size(this->type); }
    /** @return             Alignment for this parameter type. */
    size_t alignment() const { return alignment(this->type); }
    /** @return             GLSL type for this parameter type. */
    const char *glslType() const { return glslType(this->type); }
    /** @return             Whether the type of the parameter is a texture type. */
    bool isTexture() const { return isTexture(this->type); }

    static size_t size(Type type);
    static size_t alignment(Type type);
    static const char *glslType(Type type);

    /** Check whether a type is a texture type.
     * @param type          Type to check.
     * @return              Whether a type is a texture type. */
    static bool isTexture(Type type) {
        switch (type) {
            case Type::kTexture2D:
            case Type::kTextureCube:
                return true;
            default:
                return false;
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
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kInt;
    static constexpr size_t kAlignment = 4;
};

template <>
struct ShaderParameterTypeTraits<uint32_t> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kUnsignedInt;
    static constexpr size_t kAlignment = 4;
};

template <>
struct ShaderParameterTypeTraits<float> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kFloat;
    static constexpr size_t kAlignment = 4;
};

template <>
struct ShaderParameterTypeTraits<glm::vec2> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kVec2;
    static constexpr size_t kAlignment = 8;
};

template <>
struct ShaderParameterTypeTraits<glm::vec3> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kVec3;
    static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::vec4> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kVec4;
    static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::mat2> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kMat2;
    static constexpr size_t kAlignment = 8;
};

template <>
struct ShaderParameterTypeTraits<glm::mat3> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kMat3;
    static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::mat4> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kMat4;
    static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::ivec2> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kIntVec2;
    static constexpr size_t kAlignment = 8;
};

template <>
struct ShaderParameterTypeTraits<glm::ivec3> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kIntVec3;
    static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::ivec4> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kIntVec4;
    static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<ReferencePtr<Texture2D>> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kTexture2D;
};

template <>
struct ShaderParameterTypeTraits<ReferencePtr<TextureCube>> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::Type::kTextureCube;
};
