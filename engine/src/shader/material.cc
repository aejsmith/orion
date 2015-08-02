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

#include "shader/material.h"

/** Create a new material.
 * @param shader        Shader to use for the material. */
Material::Material(Shader *shader) :
    m_shader(shader),
    m_uniforms(nullptr)
{
    if (shader->uniformStruct()) {
        /* Material parameters should be changed infrequently, therefore set
         * the uniform buffer usage as static. */
        m_uniforms = new UniformBufferBase(*shader->uniformStruct(), GPUBuffer::kStaticDrawUsage);
    }
}

/** Destroy the material. */
Material::~Material() {
    delete m_uniforms;
}

/** Get a parameter value.
 * @param name          Name of the parameter to get.
 * @param type          Type of the parameter.
 * @param buf           Where to store parameter value. */
void Material::value(const char *name, ShaderParameter::Type type, void *buf) const {
    const ShaderParameter *param = m_shader->lookupParameter(name);
    checkMsg(param, "Parameter '%s' in '%s' not found", name, m_shader->path().c_str());
    checkMsg(param->type == type, "Incorrect type for parameter '%s' in '%s'", name, m_shader->path().c_str());

    if (param->type == ShaderParameter::kTextureType) {
        check(param->textureSlot <= TextureSlots::kMaterialTexturesEnd);
        new(buf) TextureBasePtr(m_textures[param->textureSlot]);
    } else {
        m_uniforms->readMember(param->uniformMember, buf);
    }
}

/** Set a parameter value.
 * @param name          Name of the parameter to set.
 * @param type          Type of the parameter.
 * @param buf           Buffer containing new parameter value. */
void Material::setValue(const char *name, ShaderParameter::Type type, const void *buf) {
    const ShaderParameter *param = m_shader->lookupParameter(name);
    checkMsg(param, "Parameter '%s' in '%s' not found", name, m_shader->path().c_str());
    checkMsg(param->type == type, "Incorrect type for parameter '%s' in '%s'", name, m_shader->path().c_str());

    if (param->type == ShaderParameter::kTextureType) {
        check(param->textureSlot <= TextureSlots::kMaterialTexturesEnd);
        m_textures[param->textureSlot] = *reinterpret_cast<const TextureBasePtr *>(buf);
    } else {
        m_uniforms->writeMember(param->uniformMember, buf);
    }
}
