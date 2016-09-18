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

#pragma once

#include "render_pass.h"

#include "core/hash_table.h"

struct VulkanFrame;

/** Vulkan pipeline implementation. */
class VulkanPipeline : public GPUPipeline, public VulkanObject {
public:
    /** Key from render state to a pipeline object. */
    struct StateKey {
        /**
         * Vertex data layout descriptor (VkPipelineVertexInputStateCreateInfo).
         *
         * We use the descriptor in the key because the GPU layer doesn't cache
         * and merge identical layout objects. TODO: Maybe it should?
         */
        GPUVertexDataLayoutDesc vertexDataLayout;

        /** Primitive type (VkPipelineInputAssemblyStateCreateInfo). */
        PrimitiveType primitiveType;

        /** Render pass compatibility key. */
        VulkanRenderPassCompatibilityKey renderPass;

        /**
         * State objects.
         *
         * These are all cached by the GPU layer, we never have multiple
         * identical objects, and once they have been created they won't ever
         * be destroyed. Therefore, we can use the pointer for the key/hash.
         */

        /** Rasterizer state (VkPipelineRasterizationStateCreateInfo). */
        const GPURasterizerState *rasterizerState;
        /** Depth/stencil state (VkPipelineDepthStencilStateCreateInfo). */
        const GPUDepthStencilState *depthStencilState;
        /** Blend state (VkPipelineColorBlendStateCreateInfo). */
        const GPUBlendState *blendState;

        StateKey(const VulkanFrame &frame, PrimitiveType primType, const GPUVertexData *vertices);

        bool operator ==(const StateKey &other) const;

        friend size_t hashValue(const StateKey &key);
    };

    VulkanPipeline(VulkanGPUManager *manager, GPUPipelineDesc &&desc);
    ~VulkanPipeline();

    VkPipeline lookup(const VulkanFrame &frame, PrimitiveType primType, const GPUVertexData *vertices);

    bool isCompatibleForSet(VulkanPipeline *other, size_t set) const;

    /** @return         Handle to the pipeline layout. */
    VkPipelineLayout layout() const { return m_layout; }
private:
    VkPipelineLayout m_layout;          /**< Pipeline layout. */

    /** Pre-created stage information. */
    std::vector<VkPipelineShaderStageCreateInfo> m_stageInfos;

    /** Hash table containing real pipeline objects. */
    HashMap<StateKey, VkPipeline> m_pipelines;

    /** Initial pipeline used to derive others from. */
    VkPipeline m_initialPipeline;
};

/**
 * Other pipeline-related objects.
 *
 * These are all GPU API type implementations that form part of pipeline state.
 * Since they are immutable, we fill out the creation information structures
 * ahead of time so we don't need to do it every time we create a new pipeline.
 */

/** Vulkan vertex data layout implementation. */
class VulkanVertexDataLayout : public GPUVertexDataLayout {
public:
    VulkanVertexDataLayout(GPUVertexDataLayoutDesc &&desc);

    /** @return             Creation information. */
    const VkPipelineVertexInputStateCreateInfo &createInfo() const { return m_createInfo; }
protected:
    ~VulkanVertexDataLayout() {}
private:
    /** Information for use with pipeline creation. */
    VkPipelineVertexInputStateCreateInfo m_createInfo;
    std::vector<VkVertexInputBindingDescription> m_bindings;
    std::vector<VkVertexInputAttributeDescription> m_attributes;
};

/** Vulkan blend state implementation. */
class VulkanBlendState : public GPUBlendState {
public:
    VulkanBlendState(const GPUBlendStateDesc &desc);

    /** @return             Creation information. */
    const VkPipelineColorBlendStateCreateInfo &createInfo() const { return m_createInfo; }
protected:
    ~VulkanBlendState() {}
private:
    /** Information for use with pipeline creation. */
    VkPipelineColorBlendStateCreateInfo m_createInfo;
    std::vector<VkPipelineColorBlendAttachmentState> m_attachments;
};

/** Vulkan depth/stencil state implementation. */
class VulkanDepthStencilState : public GPUDepthStencilState {
public:
    VulkanDepthStencilState(const GPUDepthStencilStateDesc &desc);

    /** @return             Creation information. */
    const VkPipelineDepthStencilStateCreateInfo &createInfo() const { return m_createInfo; }
protected:
    ~VulkanDepthStencilState() {}
private:
    /** Information for use with pipeline creation. */
    VkPipelineDepthStencilStateCreateInfo m_createInfo;
};

/** Vulkan rasterizer state implementation. */
class VulkanRasterizerState : public GPURasterizerState {
public:
    VulkanRasterizerState(const GPURasterizerStateDesc &desc);

    /** @return             Creation information. */
    const VkPipelineRasterizationStateCreateInfo &createInfo() const { return m_createInfo; }
protected:
    ~VulkanRasterizerState() {}
private:
    /** Information for use with pipeline creation. */
    VkPipelineRasterizationStateCreateInfo m_createInfo;
};
