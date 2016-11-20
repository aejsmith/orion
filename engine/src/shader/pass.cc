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
 *  - Improved shader variation system (preprocessor defines). Fixed sized array
 *    for light/shadow variations isn't very nice. Want a system that allows
 *    shaders to define what variations they actually support (e.g. Unity's
 *    multi_compile pragma), and materials to be able to set keywords. We would
 *    then compile for all possible combinations, and then when getting a
 *    variation, form a key by filtering selected keywords to ones supported by
 *    the shader and look up based on that.
 *  - Don't need to compile shadow variation for ambient.
 */

#include "gpu/gpu_manager.h"

#include "render/scene_light.h"

#include "render_core/render_manager.h"

#include "shader/pass.h"
#include "shader/resource.h"
#include "shader/shader.h"
#include "shader/shader_compiler.h"
#include "shader/uniform_buffer.h"

/** Array of pass variation strings, indexed by pass type. */
static const char *passShaderVariations[Pass::kNumTypes] = {
    "BASIC_PASS",
    "FORWARD_PASS",
    "DEFERRED_PASS",
    "SHADOW_CASTER_PASS",
};

/** Array of light variation strings, indexed by light type. */
static const char *lightShaderVariations[SceneLight::kNumTypes] = {
    "AMBIENT_LIGHT",
    "DIRECTIONAL_LIGHT",
    "POINT_LIGHT",
    "SPOT_LIGHT",
};

/** Shadow variation string. */
static const char *const shadowVariation = "SHADOW";

/** Initialize the pass.
 * @param parent        Parent shader.
 * @param type          Type of the pass. */
Pass::Pass(Shader *parent, Type type) :
    m_parent(parent),
    m_type(type),
    m_variations((type == Type::kForward) ? SceneLight::kNumTypes * 2 : 1)
{}

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
 * @param light         Light that the pass is being drawn with. This is used
 *                      to select which shader variations to use. It is ignored
 *                      for non-lit pass types.
 */
void Pass::setDrawState(GPUCommandList *cmdList, SceneLight *light) const {
    /* Find the variation to use. */
    size_t index;
    if (m_type == Type::kForward) {
        index = light->type() * 2;
        if (light->castShadows())
            index++;
    } else {
        index = 0;
    }

    /* Bind the variation. */
    const Variation &variation = m_variations[index];
    check(variation.pipeline);
    cmdList->bindPipeline(variation.pipeline);
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
    options.keywords = keywords;
    options.uniforms = m_parent->uniformStruct();

    /* Define texture parameters. */
    for (const auto &parameter : m_parent->parameters()) {
        if (parameter.second.isTexture())
            options.parameters.emplace_back(parameter.first, parameter.second);
    }

    /* Set pass type keyword. */
    options.keywords.insert(passShaderVariations[static_cast<size_t>(m_type)]);

    if (m_type == Type::kForward) {
        /* Build each of the light type variations. */
        for (unsigned i = 0; i < SceneLight::kNumTypes; i++) {
            for (unsigned j = 0; j < 2; j++) {
                ShaderCompiler::Options variationOptions = options;
                variationOptions.keywords.insert(lightShaderVariations[i]);
                if (j)
                    variationOptions.keywords.insert(shadowVariation);

                m_variations[(i * 2) + j].programs[stage] =
                    compileVariation(variationOptions, m_parent);
            }
        }
    } else {
        m_variations[0].programs[stage] = compileVariation(options, m_parent);
    }

    return true;
}

/** Finalise the pass (called from Shader::addPass). */
void Pass::finalise() {
    for (Variation &variation : m_variations) {
        GPUPipelineDesc pipelineDesc;

        pipelineDesc.programs = std::move(variation.programs);

        /* Bind standard resource sets. */
        const RenderManager::Resources &resources = g_renderManager->resources();
        pipelineDesc.resourceLayout.resize(ResourceSets::kNumResourceSets);
        pipelineDesc.resourceLayout[ResourceSets::kEntityResources] = resources.entityResourceSetLayout;
        pipelineDesc.resourceLayout[ResourceSets::kViewResources] = resources.viewResourceSetLayout;
        pipelineDesc.resourceLayout[ResourceSets::kLightResources] = resources.lightResourceSetLayout;
        pipelineDesc.resourceLayout[ResourceSets::kPostEffectResources] = resources.postEffectResourceSetLayout;

        /* Bind material resources. */
        pipelineDesc.resourceLayout[ResourceSets::kMaterialResources] = m_parent->m_resourceSetLayout;

        /* Create a pipeline. */
        variation.pipeline = g_gpuManager->createPipeline(std::move(pipelineDesc));
    }
}
