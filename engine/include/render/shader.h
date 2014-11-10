/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Shader classes.
 */

#pragma once

#include "engine/asset.h"

#include "render/pass.h"
#include "render/uniform_buffer.h"

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
    /** Type of the parameter map. */
    typedef std::map<std::string, ShaderParameter> ParameterMap;
public:
    ~Shader();

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
        return m_passes[type].size();
    }

    /** Get a pass.
     * @note                Does not check whether the pass exists, this must be
     *                      done manually beforehand.
     * @param type          Type of the pass to get.
     * @param index         Index of the pass.
     * @return              Pointer to pass. */
    const Pass *pass(Pass::Type type, unsigned index) const {
        return m_passes[type][index];
    }

    void setDrawState(Material *material) const;
private:
    Shader();

    void addParameter(const std::string &name, ShaderParameter::Type type);
    void addPass(Pass *pass);
private:
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

    friend class ShaderLoader;
};

/** Type of a shader pointer. */
typedef TypedAssetPtr<Shader> ShaderPtr;
