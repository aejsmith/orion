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

#include "core/serialiser.h"

#include "shader/material.h"

/** Private default constructor for deserialisation. */
Material::Material() :
    m_uniforms(nullptr)
{}

/** Create a new material.
 * @param shader        Shader to use for the material. */
Material::Material(Shader *shader) :
    m_shader(shader),
    m_uniforms(nullptr)
{
    check(shader);
    createResources();
}

/** Destroy the material. */
Material::~Material() {
    delete m_uniforms;
}

/** Create the uniform buffer. */
void Material::createResources() {
    m_resources = g_gpuManager->createResourceSet(m_shader->m_resourceSetLayout);

    if (m_shader->uniformStruct()) {
        /* Material parameters should be changed infrequently, therefore set
         * the uniform buffer usage as static. */
        m_uniforms = new UniformBufferBase(*m_shader->uniformStruct(), GPUBuffer::kStaticUsage);
        m_resources->bindUniformBuffer(ResourceSlots::kUniforms, m_uniforms->gpu());
    }
}

/** Serialise the material.
 * @param serialiser    Serialiser to write to. */
void Material::serialise(Serialiser &serialiser) const {
    fatal("Material::serialise: TODO");
}

/** Deserialise the material.
 * @param serialiser    Serialiser to read from. */
void Material::deserialise(Serialiser &serialiser) {
    serialiser.read("shader", m_shader);
    check(m_shader);

    createResources();

    if (serialiser.beginGroup("parameters")) {
        for (const auto &it : m_shader->parameters()) {
            const char *name = it.first.c_str();
            const ShaderParameter &parameter = it.second;

            // TODO: A Variant type would be great here.
            switch (parameter.type) {
                case ShaderParameter::Type::kInt:
                {
                    int value;
                    if (serialiser.read(name, value))
                        setValue(name, value);

                    break;
                }
                case ShaderParameter::Type::kUnsignedInt:
                {
                    unsigned value;
                    if (serialiser.read(name, value))
                        setValue(name, value);

                    break;
                }
                case ShaderParameter::Type::kFloat:
                {
                    float value;
                    if (serialiser.read(name, value))
                        setValue(name, value);

                    break;
                }
                case ShaderParameter::Type::kVec2:
                {
                    glm::vec2 value;
                    if (serialiser.read(name, value))
                        setValue(name, value);

                    break;
                }
                case ShaderParameter::Type::kVec3:
                {
                    glm::vec3 value;
                    if (serialiser.read(name, value))
                        setValue(name, value);

                    break;
                }
                case ShaderParameter::Type::kVec4:
                {
                    glm::vec4 value;
                    if (serialiser.read(name, value))
                        setValue(name, value);

                    break;
                }
                case ShaderParameter::Type::kTexture2D:
                {
                    Texture2DPtr value;
                    if (serialiser.read(name, value))
                        setValue(name, value);

                    break;
                }
                case ShaderParameter::Type::kTextureCube:
                {
                    TextureCubePtr value;
                    if (serialiser.read(name, value))
                        setValue(name, value);

                    break;
                }
                default:
                    check(false);
            }
        }

        serialiser.endGroup();
    }
}

/** Set shader-wide draw state for the material.
 * @param cmdList       GPU command list. */
void Material::setDrawState(GPUCommandList *cmdList) const {
    if (m_uniforms)
        m_uniforms->flush();

    cmdList->bindResourceSet(ResourceSets::kMaterialResources, m_resources);
}

/** Get a parameter value.
 * @param name          Name of the parameter to get.
 * @param type          Type of the parameter.
 * @param buf           Where to store parameter value. */
void Material::getValue(const char *name, ShaderParameter::Type type, void *buf) const {
    const ShaderParameter *param = m_shader->lookupParameter(name);
    checkMsg(param, "Parameter '%s' in '%s' not found", name, m_shader->path().c_str());
    checkMsg(param->type == type, "Incorrect type for parameter '%s' in '%s'", name, m_shader->path().c_str());

    if (param->isTexture()) {
        Asset *asset = (m_resourceAssets.size() > param->resourceSlot)
            ? m_resourceAssets[param->resourceSlot].get()
            : nullptr;

        switch (param->type) {
            case ShaderParameter::Type::kTexture2D:
                new(buf) Texture2DPtr(static_cast<Texture2D *>(asset));
                break;
            case ShaderParameter::Type::kTextureCube:
                new(buf) TextureCubePtr(static_cast<TextureCube *>(asset));
                break;
            default:
                unreachable();
        }
    } else {
        m_uniforms->readMember(param->uniformMember, buf);
    }
}

/** Set a parameter value.
 * @param name          Name of the parameter to set.
 * @param type          Type of the parameter.
 * @param buf           Buffer containing new parameter value. */
void Material::setValue(const char *name, ShaderParameter::Type type, const void *buf) {
    const ShaderParameter *parameter = m_shader->lookupParameter(name);
    checkMsg(parameter, "Parameter '%s' in '%s' not found", name, m_shader->path().c_str());
    checkMsg(parameter->type == type, "Incorrect type for parameter '%s' in '%s'", name, m_shader->path().c_str());

    if (parameter->isTexture()) {
        TextureBasePtr texture;

        switch (parameter->type) {
            case ShaderParameter::Type::kTexture2D:
                texture = *reinterpret_cast<const Texture2DPtr *>(buf);
                break;
            case ShaderParameter::Type::kTextureCube:
                texture = *reinterpret_cast<const TextureCubePtr *>(buf);
                break;
            default:
                unreachable();
        }

        m_resources->bindTexture(parameter->resourceSlot, texture->gpu(), texture->sampler());

        if (m_resourceAssets.size() <= parameter->resourceSlot)
            m_resourceAssets.resize(parameter->resourceSlot + 1);

        m_resourceAssets[parameter->resourceSlot] = std::move(texture);
    } else {
        m_uniforms->writeMember(parameter->uniformMember, buf);
    }
}

/**
 * Set a texture parameter to a GPU texture.
 *
 * This function allows a low-level GPU texture and sampler state to be passed
 * through a material to a shader. This binds the given objects directly in the
 * Material's resource set and invalidates the current parameter value, meaning
 * getValue() will return a null pointer. Any call to setValue() will invalidate
 * this binding.
 *
 * @param name          Name of the parameter to set (must be a texture).
 * @param texture       Texture to bind.
 * @param sampler       Sampler state to use.
 */
void Material::setGPUTexture(const char *name, GPUTexture *texture, GPUSamplerState *sampler) {
    const ShaderParameter *parameter = m_shader->lookupParameter(name);
    checkMsg(parameter, "Parameter '%s' in '%s' not found", name, m_shader->path().c_str());

    ShaderParameter::Type expected;

    switch (texture->type()) {
        case GPUTexture::kTexture2D:
            expected = ShaderParameter::Type::kTexture2D;
            break;
        case GPUTexture::kTextureCube:
            expected = ShaderParameter::Type::kTextureCube;
            break;
        default:
            check(false);
            return;
    }

    checkMsg(
        parameter->type == expected,
        "Incorrect type for parameter '%s' in '%s'", name, m_shader->path().c_str());

    if (m_resourceAssets.size() > parameter->resourceSlot)
        m_resourceAssets[parameter->resourceSlot] = nullptr;

    m_resources->bindTexture(parameter->resourceSlot, texture, sampler);
}
