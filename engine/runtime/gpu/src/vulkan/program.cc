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

#include "manager.h"
#include "program.h"

/** Initialise a Vulkan program from a SPIR-V binary.
 * @param manager       Manager that owns this program.
 * @param desc          Descriptor for the program. */
VulkanProgram::VulkanProgram(VulkanGPUManager *manager, GPUProgramDesc &&desc) :
    GPUProgram(desc.stage),
    VulkanHandle(manager)
{
    /* TODO: Object names. */
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = desc.spirv.size() * sizeof(desc.spirv[0]);
    createInfo.pCode = &desc.spirv[0];

    checkVk(vkCreateShaderModule(manager->device()->handle(),
                                 &createInfo,
                                 nullptr,
                                 &m_handle));
}

/** Destroy the program. */
VulkanProgram::~VulkanProgram() {
    vkDestroyShaderModule(manager()->device()->handle(), m_handle, nullptr);
}

/** Create a GPU program from a SPIR-V binary.
 * @param desc          Descriptor for the program.
 * @return              Pointer to created shader. */
GPUProgramPtr VulkanGPUManager::createProgram(GPUProgramDesc &&desc) {
    return new VulkanProgram(this, std::move(desc));
}
