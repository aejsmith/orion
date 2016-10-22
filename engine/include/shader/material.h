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
 * @brief               Material class.
 */

#pragma once

#include "engine/texture.h"

#include "gpu/resource.h"

#include "shader/shader.h"

class GPUCommandList;
class UniformBufferBase;

/**
 * Material class.
 *
 * A material is used to define the look of something when it is rendered. It
 * holds a reference to a shader and a set of parameters to the shader.
 */
class Material : public Asset {
public:
    CLASS();

    explicit Material(Shader *shader);

    /** @return             Shader for the material. */
    Shader *shader() const { return m_shader; }

    void setDrawState(GPUCommandList *cmdList) const;

    /**
     * Parameter value access.
     */

    void getValue(const char *name, ShaderParameter::Type type, void *buf) const;
    void setValue(const char *name, ShaderParameter::Type type, const void *buf);

    /** Get a parameter value.
     * @tparam T            Type of the parameter.
     * @param name          Name of the parameter to get.
     * @param value         Where to store parameter value. */
    template <typename T>
    void getValue(const char *name, T &value) const {
        getValue(name, ShaderParameterTypeTraits<T>::kType, std::addressof(value));
    }

    /** Set a parameter value.
     * @tparam T            Type of the parameter.
     * @param name          Name of the parameter to set.
     * @param value         Value to set to. */
    template <typename T>
    void setValue(const char *name, const T &value) {
        setValue(name, ShaderParameterTypeTraits<T>::kType, std::addressof(value));
    }

    void setGPUTexture(const char *name, GPUTexture *texture, GPUSamplerState *sampler);
protected:
    Material();
    ~Material();

    void serialise(Serialiser &serialiser) const override;
    void deserialise(Serialiser &serialiser) override;

    void createResources();
private:
    ShaderPtr m_shader;             /**< Shader being used by the material. */
    UniformBufferBase *m_uniforms;  /**< Uniform buffer containing material parameters. */

    /** Resource bindings for the material. */
    GPUResourceSetPtr m_resources;

    /**
     * Array of resource assets.
     *
     * Although GPUResourceSet maintains references to the underlying GPU
     * resources, if these are owned by a high level asset (e.g. Texture*) we
     * additionally need to hold a reference to that, both to keep it alive and
     * so that we can return it from getValue(). These are stored here, indexed
     * by slot number. We know their real type from the parameter type.
     */
    std::vector<AssetPtr> m_resourceAssets;

    friend class Shader;
};

/** Type of a material pointer. */
using MaterialPtr = TypedAssetPtr<Material>;
