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
 * @brief               Vulkan pipeline implementation.
 */

#include "manager.h"
#include "pipeline.h"

/** Create a pipeline object.
 * @param manager       Manager that the pipeline is for.
 * @param desc          Descriptor for the pipeline. */
VulkanPipeline::VulkanPipeline(VulkanGPUManager *manager, GPUPipelineDesc &&desc) :
    GPUPipeline(std::move(desc)),
    VulkanObject(manager)
{
    /* Create a pipeline layout. */
    std::vector<VkDescriptorSetLayout> setLayouts;
    setLayouts.reserve(m_resourceLayout.size());
    for (const GPUResourceSetLayout *it : m_resourceLayout) {
        auto resourceSetLayout = static_cast<const VulkanResourceSetLayout *>(it);
        setLayouts.push_back(resourceSetLayout->handle());
    }

    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = setLayouts.size();
    layoutCreateInfo.pSetLayouts = &setLayouts[0];

    checkVk(vkCreatePipelineLayout(manager->device()->handle(), &layoutCreateInfo, nullptr, &m_layout));
}

/** Destroy the pipeline. */
VulkanPipeline::~VulkanPipeline() {
    vkDestroyPipelineLayout(manager()->device()->handle(), m_layout, nullptr);
}

/** Create a pipeline object.
 * @param desc          Descriptor for the pipeline.
 * @return              Pointer to created pipeline. */
GPUPipelinePtr VulkanGPUManager::createPipeline(GPUPipelineDesc &&desc) {
    return new VulkanPipeline(this, std::move(desc));
}
