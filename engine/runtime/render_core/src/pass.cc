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
 * @brief               Shader pass class.
 *
 * TODO:
 *  - Cache of loaded shaders, identify ones which are identical and match them
 *    (e.g. ones which are the same despite not being compiled with the same
 *    keywords. Loading code would move from here to the shader cache.
 */

#include "gpu/gpu_manager.h"

#include "render_core/pass.h"
#include "render_core/render_resources.h"
#include "render_core/shader.h"
#include "render_core/shader_compiler.h"
#include "render_core/uniform_buffer.h"

/** Name of the basic pass type. */
const char *const Pass::kBasicType = "Basic";

/** Define the basic pass type. */
DEFINE_PASS_TYPE("Basic", PassType::VariationList());

/** Get a variation string.
 * @param variation     Variation to get for.
 * @return              Variation string. */
static std::string getVariationString(const ShaderKeywordSet &variation) {
    std::string str;

    /* TODO: This could use some optimisation. Rather than doing this every time
     * we want to look up a variation, pre-calculate the string (and a hash?) in
     * some VariationKey object. */
    for (const std::string &keyword : variation) {
        if (!str.empty())
            str += " ";
        str += keyword;
    }

    return str;
}

/** Initialize the pass.
 * @param parent        Parent shader.
 * @param type          Type of the pass. */
Pass::Pass(Shader *parent, const std::string &type) :
    m_parent(parent),
    m_type(PassType::lookup(type))
{
    /* Pre-create the variation map for all required variations. */
    for (const ShaderKeywordSet &variation : m_type.variations)
        m_variations.emplace(getVariationString(variation), Variation());
}

/** Destroy the pass. */
Pass::~Pass() {}

/**
 * Set pass draw state.
 *
 * Sets the draw state for this pass. Pass draw state is independent from the
 * material, therefore can be set once for all entities/materials being drawn
 * with this pass.
 *
 * @param cmdList       GPU command list.
 * @param variation     Variation of the pass to use. This should be a valid
 *                      variation for the type of the pass.
 */
void Pass::setDrawState(GPUCommandList *cmdList, const ShaderKeywordSet &variation) const {
    std::string str = getVariationString(variation);

    auto ret = m_variations.find(str);
    checkMsg(ret != m_variations.end(), "Invalid pass variation '%s'", str.c_str());

    check(ret->second.pipeline);
    cmdList->bindPipeline(ret->second.pipeline);
}

/** Compile a single variation.
 * @param options       Shader compiler options.
 * @param parent        Parent shader.
 * @return              Pointer to compiled program. */
static GPUProgramPtr compileVariation(const ShaderCompiler::Options &options, Shader *parent) {
    GPUProgramDesc desc;
    desc.stage = options.stage;

    /* Compile the shader. */
    if (!ShaderCompiler::compile(options, desc.spirv))
        return nullptr;

    /* Create a name string. */
    desc.name = parent->path();
    desc.name += " (";
    bool first = true;
    for (const std::string &keyword : options.keywords) {
        if (!first)
            desc.name += ", ";
        first = false;
        desc.name += keyword;
    }
    desc.name += ")";

    /* Create a GPU program. */
    return g_gpuManager->createProgram(std::move(desc));
}

/** Add a GPU shader to the pass.
 * @param stage         Stage to add this shader to.
 * @param path          Filesystem path to shader source.
 * @param keywords      Set of shader variation keywords.
 * @return              Whether the stage was loaded successfully. */
bool Pass::loadStage(unsigned stage, const Path &path, const ShaderKeywordSet &keywords) {
    ShaderCompiler::Options options;
    options.path = path;
    options.stage = stage;
    options.uniforms = m_parent->uniformStruct();

    /* Define texture parameters. */
    for (const auto &parameter : m_parent->parameters()) {
        if (parameter.second.isTexture())
            options.parameters.emplace_back(parameter.first, parameter.second);
    }

    /* Compile each variation. */
    for (const ShaderKeywordSet &variation : m_type.variations) {
        options.keywords = keywords;
        options.keywords.insert(variation.begin(), variation.end());

        m_variations[getVariationString(variation)].programs[stage] = compileVariation(options, m_parent);
    }

    return true;
}

/** Finalise the pass (called from Shader::addPass). */
void Pass::finalise() {
    for (auto &it : m_variations) {
        Variation &variation = it.second;

        GPUPipelineDesc pipelineDesc;

        pipelineDesc.programs = std::move(variation.programs);

        /* Bind standard resource sets. TODO: This should be specified by the
         * pass type. */
        pipelineDesc.resourceLayout.resize(ResourceSets::kNumResourceSets);
        pipelineDesc.resourceLayout[ResourceSets::kEntityResources] = g_renderResources->entityResourceSetLayout();
        pipelineDesc.resourceLayout[ResourceSets::kViewResources] = g_renderResources->viewResourceSetLayout();
        pipelineDesc.resourceLayout[ResourceSets::kLightResources] = g_renderResources->lightResourceSetLayout();
        pipelineDesc.resourceLayout[ResourceSets::kPostEffectResources] = g_renderResources->postEffectResourceSetLayout();

        /* Bind material resources. */
        pipelineDesc.resourceLayout[ResourceSets::kMaterialResources] = m_parent->m_resourceSetLayout;

        /* Create a pipeline. */
        variation.pipeline = g_gpuManager->createPipeline(std::move(pipelineDesc));
    }
}

/** @return             Global pass type map. */
static auto &passTypeMap() {
    static HashMap<std::string, PassType *> map;
    return map;
}

/** Register the pass type.
 * @param name          Pass type name.
 * @param variations    List of variations to compile, i.e. a list of different
 *                      combinations of keywords. An empty list will result in
 *                      1 variation being compiled with no additional keywords. */
PassType::PassType(std::string inName, VariationList inVariations) :
    name(std::move(inName)),
    variations(std::move(inVariations))
{
    /* Add a single variation with no keywords if the list is empty. */
    if (this->variations.empty())
        this->variations.emplace_back();

    auto ret = passTypeMap().emplace(name, this);
    checkMsg(ret.second, "Duplicate pass type");
}

/** Unregister the pass type. */
PassType::~PassType() {
    passTypeMap().erase(this->name);
}

/** Look up a pass type.
 * @param name          Pass type name.
 * @return              Reference to the pass type. */
const PassType &PassType::lookup(const std::string &name) {
    const auto &map = passTypeMap();

    auto ret = map.find(name);
    if (ret == map.end())
        fatal("Unknown pass type '%s'", name.c_str());

    return *ret->second;
}
