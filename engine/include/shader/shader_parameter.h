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
 * @brief               Shader parameter definitions.
 */

#pragma once

#include "core/refcounted.h"

class TextureBase;

struct UniformStructMember;

/** Details of a shader parameter. */
struct ShaderParameter {
    /** Enumeration of parameter types. */
    enum Type {
        /** Basic types. */
        kIntType,                   /**< Signed 32-bit integer. */
        kUnsignedIntType,           /**< Unsigned 32-bit integer. */
        kFloatType,                 /**< Single-precision floating point. */
        kVec2Type,                  /**< 2 component floating point vector. */
        kVec3Type,                  /**< 3 component floating point vector. */
        kVec4Type,                  /**< 4 component floating point vector. */
        kMat2Type,                  /**< 2x2 floating point matrix. */
        kMat3Type,                  /**< 3x3 floating point matrix. */
        kMat4Type,                  /**< 4x4 floating point matrix. */
        kIntVec2Type,               /**< 2 component integer vector. */
        kIntVec3Type,               /**< 3 component integer vector. */
        kIntVec4Type,               /**< 4 component integer vector. */

        /** Special types (cannot be used in uniform structures). */
        kTextureType,               /**< Texture. */
    };

    Type type;                      /**< Parameter type. */

    union {
        /** For uniform parameters, the struct member for the parameter. */
        const UniformStructMember *uniformMember;
        /** For texture parameters, the texture slot to bind to. */
        unsigned textureSlot;
    };
public:
    /** @return             Storage size of the parameter. */
    size_t size() const { return size(this->type); }
    /** @return             Alignment for this parameter type. */
    size_t alignment() const { return alignment(this->type); }
    /** @return             GLSL type for this parameter type. */
    const char *glslType() const { return glslType(this->type); }

    static size_t size(Type type);
    static size_t alignment(Type type);
    static const char *glslType(Type type);
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

template <>
struct ShaderParameterTypeTraits<glm::ivec2> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::kIntVec2Type;
    static constexpr size_t kAlignment = 8;
};

template <>
struct ShaderParameterTypeTraits<glm::ivec3> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::kIntVec3Type;
    static constexpr size_t kAlignment = 16;
};

template <>
struct ShaderParameterTypeTraits<glm::ivec4> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::kIntVec4Type;
    static constexpr size_t kAlignment = 16;
};

/**
 * Texture2D is specifically left unimplemented here because at the moment we
 * don't type parameters to a specific texture type, just generic types.
 * Implementing Texture2D here makes Material::value() unsafe.
 */
template <>
struct ShaderParameterTypeTraits<ReferencePtr<TextureBase>> {
    static constexpr ShaderParameter::Type kType = ShaderParameter::kTextureType;
};
