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

#include "pipeline.h"

/** Create a pipeline object.
 * @param manager       Manager that the pipeline is for.
 * @param desc          Descriptor for the pipeline. */
VulkanPipeline::VulkanPipeline(VulkanGPUManager *manager, GPUPipelineDesc &&desc) :
    GPUPipeline(std::move(desc)),
    VulkanObject(manager)
{}

/** Destroy the pipeline. */
VulkanPipeline::~VulkanPipeline() {}

/** Create a pipeline object.
 * @param desc          Descriptor for the pipeline.
 * @return              Pointer to created pipeline. */
GPUPipelinePtr VulkanGPUManager::createPipeline(GPUPipelineDesc &&desc) {
    return new VulkanPipeline(this, std::move(desc));
}
