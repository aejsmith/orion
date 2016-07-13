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
 * @brief               Shader classes.
 */

#pragma once

#include "engine/asset.h"

#include "shader/pass.h"
#include "shader/uniform_buffer.h"

#include <array>
#include <map>
#include <vector>

class Material;

/**
 * Shader class.
 *
 * This class implements the CPU side of a shader. A shader defines a set of
 * parameters, and a set of rendering passes required to achieve the desired
 * effect. A pass defines the actual GPU shaders that will be used and other
 * bits of GPU state. Parameter values are supplied to shaders via Materials.
 *
 * A shader can have uniform parameters, which are automatically filled into
 * a uniform buffer and made available to GPU shaders in the kMaterialUniforms
 * uniform buffer slot, and texture parameters, which are made available to GPU
 * shaders in the specified texture slot.
 */
class Shader : public Asset {
public:
    CLASS();

    /** Type of the parameter map. */
    using ParameterMap = std::map<std::string, ShaderParameter>;

    /** @return             Uniform structure used by the shader. */
    const UniformStruct *uniformStruct() const { return m_uniformStruct; }
    /** @return             Parameter map for the shader. */
    const ParameterMap &parameters() const { return m_parameters; }
    /** @return             Number of texture parameters. */
    unsigned numTextures() const { return m_nextTextureSlot; }

    const ShaderParameter *lookupParameter(const std::string &name) const;

    /** Get the number of passes of a certain type the shader has.
     * @param type          Type to get.
     * @return              Number of passes of the specified type. */
    size_t numPasses(Pass::Type type) const {
        return m_passes[static_cast<size_t>(type)].size();
    }

    /** Get a pass.
     * @note                Does not check whether the pass exists, this must be
     *                      done manually beforehand.
     * @param type          Type of the pass to get.
     * @param index         Index of the pass.
     * @return              Pointer to pass. */
    const Pass *pass(Pass::Type type, unsigned index) const {
        return m_passes[static_cast<size_t>(type)][index];
    }

    void setDrawState(Material *material) const;
protected:
    ~Shader();

    void serialise(Serialiser &serialiser) const override;
    void deserialise(Serialiser &serialiser) override;
private:
    Shader();

    void addParameter(const std::string &name, ShaderParameter::Type type);
    void addPass(Pass *pass);

    UniformStruct *m_uniformStruct;     /**< Uniform structure used by the shader. */
    ParameterMap m_parameters;          /**< Map of registered parameters. */
    unsigned m_nextTextureSlot;         /**< Next available texture slot. */

    /**
     * Array of passes.
     *
     * Array for the different pass types, with a variable-sized array
     * within that for all passes of that type.
     */
    std::array<std::vector<Pass *>, Pass::kNumTypes> m_passes;
};

/** Type of a shader pointer. */
using ShaderPtr = TypedAssetPtr<Shader>;
