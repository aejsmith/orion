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
 * @brief               Vulkan GPU manager.
 */

#pragma once

/** Whether to enable the Vulkan validation layers. */
#define ORION_VULKAN_VALIDATION 1

#include "loader.h"

#include "core/hash_table.h"

#include "gpu/gpu_manager.h"

/** Macro to check the result of Vulkan API calls. */
#define checkVk(call) \
    { \
        VkResult __result = call; \
        if (unlikely(__result != VK_SUCCESS)) \
            fatal("Vulkan call failed: %d", __result); \
    }

class VulkanDevice;
class VulkanSurface;
class VulkanSwapchain;

/** Details of Vulkan features. */
struct VulkanFeatures {
    bool validation;                    /**< Whether validation layers are enabled. */
};

/** Vulkan GPU manager implementation. */
class VulkanGPUManager : public GPUManager {
public:
    VulkanGPUManager(const EngineConfiguration &config, Window *&window);
    ~VulkanGPUManager();

    /**
     * GPUManager interface.
     */

    GPUBlendStatePtr createBlendState(const GPUBlendStateDesc &desc) override;
    GPUBufferPtr createBuffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) override;
    GPUDepthStencilStatePtr createDepthStencilState(const GPUDepthStencilStateDesc &desc) override;
    GPUIndexDataPtr createIndexData(
        GPUBuffer *buffer,
        GPUIndexData::Type type,
        size_t count,
        size_t offset) override;
    GPUPipelinePtr createPipeline(const GPUPipelineDesc &desc) override;
    GPURasterizerStatePtr createRasterizerState(const GPURasterizerStateDesc &desc) override;
    GPUSamplerStatePtr createSamplerState(const GPUSamplerStateDesc &desc) override;
    GPUTexturePtr createTexture(const GPUTextureDesc &desc) override;
    GPUTexturePtr createTextureView(const GPUTextureImageRef &image) override;
    GPUVertexDataPtr createVertexData(size_t count, GPUVertexFormat *format, GPUBufferArray &buffers) override;
    GPUVertexFormatPtr createVertexFormat(VertexBufferLayoutArray &buffers, VertexAttributeArray &attributes) override;

    GPUProgramPtr compileProgram(unsigned stage, const std::string &source) override;

    void bindPipeline(GPUPipeline *pipeline) override;
    void bindTexture(unsigned index, GPUTexture *texture, GPUSamplerState *sampler) override;
    void bindUniformBuffer(unsigned index, GPUBuffer *buffer) override;
    void setBlendState(GPUBlendState *state) override;
    void setDepthStencilState(GPUDepthStencilState *state) override;
    void setRasterizerState(GPURasterizerState *state) override;
    void setRenderTarget(const GPURenderTargetDesc *desc, const IntRect *viewport) override;
    void setViewport(const IntRect &viewport) override;
    void setScissor(bool enable, const IntRect &scissor) override;

    void startFrame() override;
    void endFrame() override;

    void blit(
        const GPUTextureImageRef &source,
        const GPUTextureImageRef &dest,
        glm::ivec2 sourcePos,
        glm::ivec2 destPos,
        glm::ivec2 size) override;
    void clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil) override;
    void draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) override;

    /**
     * Internal definitions.
     */

    /** @return             Feature details structure. */
    const VulkanFeatures &features() const { return m_features; }
    /** @return             Vulkan instance handle. */
    VkInstance instance() const { return m_instance; }
    /** @return             Instance extension function table. */
    const VulkanInstanceFunctions &functions() const { return m_functions; }
    /** @return             Surface for the main window. */
    VulkanSurface *surface() const { return m_surface; }
    /** @return             Main logical device. */
    VulkanDevice *device() const { return m_device; }
private:
    VulkanFeatures m_features;              /**< Feature details. */
    VkInstance m_instance;                  /**< Vulkan instance handle. */
    VulkanInstanceFunctions m_functions;    /**< Instance function pointer table. */
    VulkanSurface *m_surface;               /**< Surface for the main window. */
    VulkanDevice *m_device;                 /**< Main logical device. */
    VulkanSwapchain *m_swapchain;           /**< Swap chain. */
};

extern VulkanGPUManager *g_vulkan;
