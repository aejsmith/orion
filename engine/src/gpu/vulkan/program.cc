/*
 * Copyright (C) 2016 Alex Smith
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
 * @brief               Vulkan GPU program implementation.
 */

#include "device.h"
#include "program.h"

/** Initialise a Vulkan program from a SPIR-V binary.
 * @param manager       Manager that owns this program.
 * @param stage         Stage that the program is for.
 * @param spirv         SPIR-V binary for the shader.
 * @param name          Name of the program for debugging purposes. */
VulkanProgram::VulkanProgram(
    VulkanGPUManager *manager,
    unsigned stage,
    const std::vector<uint32_t> &spirv,
    const std::string &name)
    :
    GPUProgram(stage),
    VulkanHandle(manager)
{
    /* TODO: Object names. */
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(spirv[0]);
    createInfo.pCode = &spirv[0];

    checkVk(vkCreateShaderModule(
        manager->device()->handle(),
        &createInfo,
        nullptr,
        &m_handle));
}

/** Destroy the program. */
VulkanProgram::~VulkanProgram() {
    vkDestroyShaderModule(manager()->device()->handle(), m_handle, nullptr);
}

/** Create a GPU program from a SPIR-V binary.
 * @param stage         Stage that the program is for.
 * @param spirv         SPIR-V binary for the shader.
 * @param name          Name of the program for debugging purposes.
 * @return              Pointer to created shader on success, null on error. */
GPUProgramPtr VulkanGPUManager::createProgram(
    unsigned stage,
    const std::vector<uint32_t> &spirv,
    const std::string &name)
{
    return new VulkanProgram(this, stage, spirv, name);
}
