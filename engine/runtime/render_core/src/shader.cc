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
 * @brief               Shader class.
 */

#include "engine/serialiser.h"

#include "render_core/material.h"
#include "render_core/pass.h"
#include "render_core/shader.h"

/** Initialize the shader. */
Shader::Shader() :
    m_uniformStruct (nullptr)
{}

/** Destroy the shader. */
Shader::~Shader() {
    /* Delete all passes. */
    for (const auto &it : m_passes) {
        for (Pass *pass : it.second)
            delete pass;
    }

    delete m_uniformStruct;
}

/** Serialise the shader.
 * @param serialiser    Serialiser to write to. */
void Shader::serialise(Serialiser &serialiser) const {
    fatal("Shader::serialise: TODO");
}

/** Deserialise the shader.
 * @param serialiser    Serialiser to read from. */
void Shader::deserialise(Serialiser &serialiser) {
    if (serialiser.beginArray("parameters")) {
        while (serialiser.beginGroup()) {
            std::string name;
            serialiser.read("name", name);
            check(!name.empty());

            ShaderParameter::Type type;
            bool hasType = serialiser.read("type", type);
            check(hasType);

            addParameter(name, type);

            serialiser.endGroup();
        }

        serialiser.endArray();
    }

    finaliseParameters();

    if (serialiser.beginArray("passes")) {
        while (serialiser.beginGroup()) {
            std::string type;
            bool hasType = serialiser.read("type", type);
            check(hasType);

            std::unique_ptr<Pass> pass(new Pass(this, type));

            auto deserialiseStage =
                [&] (const char *name, unsigned stage) {
                    if (!serialiser.beginGroup(name))
                        return false;

                    std::string path;
                    serialiser.read("source", path);
                    check(!path.empty());

                    ShaderKeywordSet keywords;

                    if (serialiser.beginArray("keywords")) {
                        std::string keyword;
                        while (serialiser.pop(keyword))
                            keywords.insert(keyword);

                        serialiser.endArray();
                    }

                    bool ret = pass->loadStage(stage, path, keywords);
                    serialiser.endGroup();
                    return ret;
                };

            bool hasVertex = deserialiseStage("vertex", ShaderStage::kVertex);
            check(hasVertex);
            bool hasFragment = deserialiseStage("fragment", ShaderStage::kFragment);
            check(hasFragment);

            addPass(pass.release());

            serialiser.endGroup();
        }

        serialiser.endArray();
    }
}

/** Create a resource set layout after adding all parameters. */
void Shader::finaliseParameters() {
    // TODO: This is fine for now, but if in future we want dynamic modification
    // of shader parameters (e.g. in an editor) we will need to recreate this
    // as needed, and then recreate all material resource sets from it.

    GPUResourceSetLayoutDesc desc(1);
    unsigned nextSlot = 1;

    for (auto &it : m_parameters) {
        const std::string &name = it.first;
        ShaderParameter &parameter = it.second;

        if (parameter.isTexture()) {
            /* Assign a resource slot. */
            parameter.resourceSlot = nextSlot++;
            desc.slots.resize(nextSlot);
            desc.slots[parameter.resourceSlot].type = GPUResourceType::kTexture;
        } else {
            /* Add a uniform struct member for it. Create struct if we don't
             * already have one. */
            if (!m_uniformStruct) {
                m_uniformStruct = new UniformStruct("MaterialUniforms",
                                                    nullptr,
                                                    ResourceSets::kMaterialResources);
                desc.slots[ResourceSlots::kUniforms].type = GPUResourceType::kUniformBuffer;
            }

            /* A bit nasty, UniformStructMember has a const char * for name, not
             * a std::string, so we point to the name string in the map key.
             * This avoids storing multiple copies of the name string. */
            parameter.uniformMember = m_uniformStruct->addMember(name.c_str(), parameter.type);
        }
    }

    m_resourceSetLayout = g_gpuManager->createResourceSetLayout(std::move(desc));
}

/** Add a parameter to the shader.
 * @param name          Name of the parameter to add.
 * @param type          Type of the parameter. */
void Shader::addParameter(const std::string &name, ShaderParameter::Type type) {
    auto ret = m_parameters.emplace(std::make_pair(name, ShaderParameter()));
    checkMsg(ret.second, "Adding duplicate shader parameter '%s'", name.c_str());

    ShaderParameter &parameter = ret.first->second;
    parameter.type = type;
}

/** Look up a parameter by name.
 * @param name          Name of the parameter to look up.
 * @return              Pointer to parameter if found, null if not. */
const ShaderParameter *Shader::lookupParameter(const std::string &name) const {
    auto it = m_parameters.find(name);
    return (it != m_parameters.end()) ? &it->second : nullptr;
}

/** Get the number of passes of a certain type the shader has.
 * @param type          Type to get.
 * @return              Number of passes of the specified type. */
size_t Shader::numPasses(const std::string &type) const {
    auto ret = m_passes.find(type);
    if (ret != m_passes.end()) {
        return ret->second.size();
    } else {
        return 0;
    }
}

/** Get a pass.
 * @param type          Type of the pass to get.
 * @param index         Index of the pass to get.
 * @return              Pointer to pass if found, null if not. */
const Pass *Shader::getPass(const std::string &type, size_t index) const {
    auto ret = m_passes.find(type);
    if (ret != m_passes.end()) {
        if (index < ret->second.size())
            return ret->second[index];
    }

    return nullptr;
}

/** Add a pass to the shader.
 * @param pass          Pass to add. Becomes owned by the shader, will be
 *                      deleted when the shader is destroyed. */
void Shader::addPass(Pass *pass) {
    /* Finalise the pipeline. */
    pass->finalise();

    m_passes[pass->type()].push_back(pass);
}
