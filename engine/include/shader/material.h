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
 * @brief               Material class.
 */

#pragma once

#include "engine/texture.h"

#include "shader/shader.h"
#include "shader/slots.h"

#include <array>

class UniformBufferBase;

/**
 * Material class.
 *
 * A material is used to define the look of something when it is rendered. It
 * holds a reference to a shader and a set of parameters to the shader.
 */
class Material : public Asset {
public:
    explicit Material(Shader *shader);
    ~Material();

    /** @return             Shader for the material. */
    Shader *shader() const { return m_shader; }

    /**
     * Parameter value access.
     */

    void value(const char *name, ShaderParameter::Type type, void *buf) const;
    void setValue(const char *name, ShaderParameter::Type type, const void *buf);

    /** Get a parameter value.
     * @tparam T            Type of the parameter.
     * @param name          Name of the parameter to get.
     * @return              Parameter value. */
    template <typename T> T value(const char *name) {
        T ret;
        value(name, ShaderParameterTypeTraits<T>::kType, std::addressof(ret));
        return ret;
    }

    /** Set a parameter value.
     * @tparam T            Type of the parameter.
     * @param name          Name of the parameter to set.
     * @param value         Value to set to. */
    template <typename T> void setValue(const char *name, const T &value) {
        setValue(name, ShaderParameterTypeTraits<T>::kType, std::addressof(value));
    }
private:
    /** Type of the texture array, indexed by slot. */
    typedef std::array<TextureBasePtr, TextureSlots::kMaterialTexturesEnd + 1> TextureArray;
private:
    ShaderPtr m_shader;             /**< Shader being used by the material. */
    UniformBufferBase *m_uniforms;  /**< Uniform buffer containing material parameters. */
    TextureArray m_textures;        /**< Array of textures, indexed by slot. */

    friend class Shader;
};

/** Type of a material pointer. */
typedef TypedAssetPtr<Material> MaterialPtr;