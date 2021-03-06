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

#include "gpu/resource.h"

#include "render_core/defs.h"
#include "render_core/pass.h"
#include "render_core/uniform_buffer.h"

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
 * A shader's parameters are either of basic types, or are resources. Basic
 * types are automatically filled into a uniform buffer and defined in shader
 * source code as global variables with matching names. Resources are
 * automatically assigned resource slots and defined in shader code bound to
 * the assigned slot.
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

    const ShaderParameter *lookupParameter(const std::string &name) const;

    size_t numPasses(const std::string &type) const;
    const Pass *getPass(const std::string &type, size_t index) const;
protected:
    ~Shader();

    void serialise(Serialiser &serialiser) const override;
    void deserialise(Serialiser &serialiser) override;
private:
    Shader();

    void addParameter(const std::string &name, ShaderParameter::Type type);
    void addPass(Pass *pass);

    void finaliseParameters();

    /** Map of registered parameters. */
    ParameterMap m_parameters;
    /** Uniform structure for the shader, generated from parameters. */
    UniformStruct *m_uniformStruct;
    /** Resource set layout for the shader, generated from parameters. */
    GPUResourceSetLayoutPtr m_resourceSetLayout;

    /** Map from pass type to an array of passes. */
    HashMap<std::string, std::vector<Pass *>> m_passes;

    friend class Material;
    friend class Pass;
};

/** Type of a shader pointer. */
using ShaderPtr = TypedAssetPtr<Shader>;
