/*
 * Copyright (C) 2016-2017 Alex Smith
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

#include "buffer.h"
#include "command_buffer.h"
#include "device.h"
#include "memory_manager.h"
#include "pipeline.h"
#include "program.h"
#include "queue.h"
#include "render_pass.h"
#include "resource.h"
#include "surface.h"
#include "swapchain.h"
#include "texture.h"
#include "utility.h"

#include "core/hash_table.h"

#include <list>

/** Details of Vulkan features. */
struct VulkanFeatures {
    VkPhysicalDeviceFeatures device;    /**< Physical device features. */

    /** Extensions etc. */
    bool validation : 1;                /**< Whether validation layers are enabled. */
    bool debugMarker : 1;               /**< Whether the debug marker extension is enabled. */

    /** Structure containing details of a pixel format. */
    struct Format {
        VkFormat format;                /**< Vulkan format value. */
        VkFormatProperties properties;  /**< Format properties. */
    };

    /** Array of pixel format information, indexed by generic pixel format. */
    std::array<Format, PixelFormat::kNumFormats> formats;
};

/** Structure tracking per-frame data for cleanup once the frame completes. */
struct VulkanFrame {
    /** Fence signalled upon completion of the frame's submission. */
    VulkanFence fence;

    /** Primary command buffer for the current frame. */
    VulkanCommandBuffer *primaryCmdBuf;

    /** List of command buffers allocated for the frame. */
    std::list<VulkanCommandBuffer *> cmdBuffers;

    /** List of staging memory allocations for the frame. */
    std::list<VulkanMemoryManager::StagingMemory *> stagingAllocations;

    /** Initialise the frame.
     * @param manager       Manager that owns the frame. */
    explicit VulkanFrame(VulkanGPUManager *manager) :
        fence(manager)
    {}
};

/** Vulkan GPU manager implementation. */
class VulkanGPUManager : public GPUManager {
public:
    VulkanGPUManager(const EngineConfiguration &config, Window *&window);
    ~VulkanGPUManager();

    /**
     * GPUManager interface.
     */

    GPUBufferPtr createBuffer(const GPUBufferDesc &desc) override;
    GPUPipelinePtr createPipeline(GPUPipelineDesc &&desc) override;
    GPUQueryPoolPtr createQueryPool(const GPUQueryPoolDesc &desc) override;
    GPURenderPassPtr createRenderPass(GPURenderPassDesc &&desc) override;
    GPUTexturePtr createTexture(const GPUTextureDesc &desc) override;
    GPUTexturePtr createTextureView(const GPUTextureViewDesc &desc) override;

    GPUBlendStatePtr createBlendState(const GPUBlendStateDesc &desc) override;
    GPUDepthStencilStatePtr createDepthStencilState(const GPUDepthStencilStateDesc &desc) override;
    GPURasterizerStatePtr createRasterizerState(const GPURasterizerStateDesc &desc) override;
    GPUSamplerStatePtr createSamplerState(const GPUSamplerStateDesc &desc) override;
    GPUVertexDataLayoutPtr createVertexDataLayout(const GPUVertexDataLayoutDesc &desc) override;

    GPUResourceSetLayoutPtr createResourceSetLayout(GPUResourceSetLayoutDesc &&desc) override;
    GPUResourceSetPtr createResourceSet(GPUResourceSetLayout *layout) override;
    GPUProgramPtr createProgram(GPUProgramDesc &&desc) override;

    void endFrame() override;

    void blit(const GPUTextureImageRef &source,
              const GPUTextureImageRef &dest,
              glm::ivec2 sourcePos,
              glm::ivec2 destPos,
              glm::ivec2 size) override;

    GPUCommandList *beginRenderPass(const GPURenderPassInstanceDesc &desc) override;
    void submitRenderPass(GPUCommandList *cmdList) override;

    void endQuery(GPUQueryPool *queryPool, uint32_t index) override;

    void beginDebugGroup(const std::string &str) override;
    void endDebugGroup() override;

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
    /** @return             Device's queue. */
    VulkanQueue *queue() const { return m_queue; }
    /** @return             Device's command pool. */
    VulkanCommandPool *commandPool() const { return m_commandPool; }
    /** @return             Device's descriptor pool. */
    VulkanDescriptorPool *descriptorPool() const { return m_descriptorPool; }
    /** @return             Device's memory manager. */
    VulkanMemoryManager *memoryManager() const { return m_memoryManager; }
    /** @return             Device's swapchain. */
    VulkanSwapchain *swapchain() const { return m_swapchain; }

    /** @return             Data for the current frame. */
    const VulkanFrame &currentFrame() const { return m_frames.back(); }
    /** @return             Data for the current frame. */
    VulkanFrame &currentFrame() { return m_frames.back(); }

    void flush();
    void invalidateFramebuffers(const VulkanTexture *texture);
private:
    void initFeatures();

    void startFrame();
    void cleanupFrames(bool shutdown);

    VulkanFeatures m_features;              /**< Feature details. */
    VkInstance m_instance;                  /**< Vulkan instance handle. */
    VulkanInstanceFunctions m_functions;    /**< Instance function pointer table. */
    VulkanSurface *m_surface;               /**< Surface for the main window. */
    VulkanDevice *m_device;                 /**< Main logical device. */
    VulkanQueue *m_queue;                   /**< Device queue. */
    VulkanCommandPool *m_commandPool;       /**< Command buffer pool. */
    VulkanDescriptorPool *m_descriptorPool; /**< Descriptor pool. */
    VulkanMemoryManager *m_memoryManager;   /**< Device memory manager. */
    VulkanSwapchain *m_swapchain;           /**< Swap chain. */

    /**
     * List of frame data.
     *
     * The current frame's data is the last element of the list. We have to
     * keep around resources used by earlier frames until their work has been
     * completed, which is determined using the fence. Once a frame has been
     * completed, we free up any resources used for it which are no longer
     * needed.
     */
    std::list<VulkanFrame> m_frames;

    /** Hash table of cached framebuffers. */
    HashMap<VulkanFramebufferKey, VulkanFramebuffer *> m_framebuffers;

    #if ORION_VULKAN_VALIDATION
    /** Debug report callback. */
    VkDebugReportCallbackEXT m_debugReportCallback;
    #endif
};
