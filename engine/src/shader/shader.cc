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

#include "core/serialiser.h"

#include "shader/material.h"
#include "shader/pass.h"
#include "shader/shader.h"

/** Initialize the shader. */
Shader::Shader() :
    m_uniformStruct(nullptr),
    m_nextTextureSlot(0)
{}

/** Destroy the shader. */
Shader::~Shader() {
    /* Delete all passes. */
    for (size_t i = 0; i < m_passes.size(); i++) {
        for (size_t j = 0; j < m_passes[i].size(); j++)
            delete m_passes[i][j];
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

    if (serialiser.beginArray("passes")) {
        while (serialiser.beginGroup()) {
            Pass::Type type;
            bool hasType = serialiser.read("type", type);
            check(hasType);

            if (type == Pass::Type::kDeferred)
                check(numPasses(Pass::Type::kDeferred) == 0);

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

/** Set shader-wide draw state for a material.
 * @param material      Material being rendered with. */
void Shader::setDrawState(Material *material) const {
    /* Set the uniform buffer if we have one. */
    if (m_uniformStruct) {
        UniformBufferBase *buffer = material->m_uniforms;
        check(buffer);
        g_gpuManager->bindUniformBuffer(UniformSlots::kMaterialUniforms, buffer->gpu());
    }

    /* Bind textures. */
    const Material::TextureArray &textures = material->m_textures;
    for (size_t i = 0; i < m_nextTextureSlot; i++) {
        if (textures[i])
            g_gpuManager->bindTexture(i, textures[i]->gpu(), textures[i]->sampler());
    }
}

/** Add a parameter to the shader.
 * @param name          Name of the parameter to add.
 * @param type          Type of the parameter. */
void Shader::addParameter(const std::string &name, ShaderParameter::Type type) {
    auto ret = m_parameters.emplace(std::make_pair(name, ShaderParameter()));
    checkMsg(ret.second, "Adding duplicate shader parameter '%s'", name.c_str());

    ShaderParameter &param = ret.first->second;
    param.type = type;

    if (ShaderParameter::isTexture(type)) {
        /* Assign a texture slot. */
        checkMsg(
            m_nextTextureSlot <= TextureSlots::kMaterialTexturesEnd,
            "Parameter '%s' exceeds maximum number of textures", name.c_str());

        param.textureSlot = m_nextTextureSlot++;
    } else {
        /* Add a uniform struct member for it. Create struct if we don't already
         * have one. */
        if (!m_uniformStruct)
            m_uniformStruct = new UniformStruct("MaterialUniforms", nullptr, UniformSlots::kMaterialUniforms);

        /* A bit nasty, UniformStructMember has a char * for name, not a
         * std::string, so we point to the name string in the map key. This
         * avoids storing multiple copies of the name string. */
        param.uniformMember = m_uniformStruct->addMember(ret.first->first.c_str(), type);
    }
}

/** Look up a parameter by name.
 * @param name          Name of the parameter to look up.
 * @return              Pointer to parameter if found, null if not. */
const ShaderParameter *Shader::lookupParameter(const std::string &name) const {
    auto it = m_parameters.find(name);
    return (it != m_parameters.end()) ? &it->second : nullptr;
}

/** Add a pass to the shader.
 * @param pass          Pass to add. Becomes owned by the shader, will be
 *                      deleted when the shader is destroyed. */
void Shader::addPass(Pass *pass) {
    size_t index = static_cast<size_t>(pass->type());

    switch (pass->type()) {
        case Pass::Type::kDeferred:
            checkMsg(
                m_passes[index].size() == 0,
                "Only one deferred pass is allowed per shader");
            break;
        default:
            break;
    }

    /* Finalize the pipeline. */
    pass->finalize();

    m_passes[index].push_back(pass);
}
