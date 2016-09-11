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

#include <list>

/** Macro to check the result of Vulkan API calls. */
#define checkVk(call) \
    { \
        VkResult __result = call; \
        if (unlikely(__result != VK_SUCCESS)) \
            fatal("Vulkan call failed: %d", __result); \
    }

class VulkanCommandBuffer;
class VulkanCommandPool;
class VulkanDevice;
class VulkanGPUManager;
class VulkanMemoryManager;
class VulkanQueue;
class VulkanSurface;
class VulkanSwapchain;

struct VulkanFrame;

/** Details of Vulkan features. */
struct VulkanFeatures {
    bool validation;                    /**< Whether validation layers are enabled. */

    /** Structure containing details of a pixel format. */
    struct Format {
        VkFormat format;                /**< Vulkan format value. */
        VkFormatProperties properties;  /**< Format properties. */
    };

    /** Array of pixel format information, indexed by generic pixel format. */
    std::array<Format, PixelFormat::kNumFormats> formats;
};

/** Vulkan GPU manager implementation. */
class VulkanGPUManager : public GPUManager {
public:
    VulkanGPUManager(const EngineConfiguration &config, Window *&window);
    ~VulkanGPUManager();

    /**
     * GPUManager interface.
     */

    GPUBufferPtr createBuffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) override;
    GPUPipelinePtr createPipeline(GPUPipelineDesc &&desc) override;
    GPURenderPassPtr createRenderPass(GPURenderPassDesc &&desc) override;
    GPUTexturePtr createTexture(const GPUTextureDesc &desc) override;
    GPUTexturePtr createTextureView(const GPUTextureImageRef &image) override;

    GPUSamplerStatePtr createSamplerState(const GPUSamplerStateDesc &desc) override;

    GPUResourceSetLayoutPtr createResourceSetLayout(GPUResourceSetLayoutDesc &&desc) override;
    GPUResourceSetPtr createResourceSet(GPUResourceSetLayout *layout) override;
    GPUProgramPtr createProgram(unsigned stage, const std::vector<uint32_t> &spirv, const std::string &name) override;

    void endFrame() override;

    void blit(
        const GPUTextureImageRef &source,
        const GPUTextureImageRef &dest,
        glm::ivec2 sourcePos,
        glm::ivec2 destPos,
        glm::ivec2 size) override;

    void beginRenderPass(const GPURenderPassInstanceDesc &desc) override;
    void endRenderPass() override;

    void bindPipeline(GPUPipeline *pipeline) override;
    void bindResourceSet(unsigned index, GPUResourceSet *resources) override;
    void setBlendState(GPUBlendState *state) override;
    void setDepthStencilState(GPUDepthStencilState *state) override;
    void setRasterizerState(GPURasterizerState *state) override;
    void setViewport(const IntRect &viewport) override;
    void setScissor(bool enable, const IntRect &scissor) override;

    void draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) override;

    #ifdef ORION_BUILD_DEBUG
    void beginDebugGroup(const std::string &str) override;
    void endDebugGroup() override;
    #endif

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
    /** @return             Device's memory manager. */
    VulkanMemoryManager *memoryManager() const { return m_memoryManager; }

    /** @return             Data for the current frame. */
    const VulkanFrame &currentFrame() const { return *m_frames.back(); }
    /** @return             Data for the current frame. */
    VulkanFrame &currentFrame() { return *m_frames.back(); }

    /**
     * Get the primary command buffer for the current frame.
     *
     * At the start of each frame, a transient command buffer is allocated to
     * act as the "primary" command buffer for the frame. This is where we
     * record everything for the frame (potentially referencing secondary
     * command buffers), and it is submitted in one go at the end of the frame.
     *
     * @return              Primary command buffer.
     */
    VulkanCommandBuffer *primaryCmdBuf() const {
        check(m_primaryCmdBuf);
        return m_primaryCmdBuf;
    }
private:
    void initFeatures();

    void startFrame();

    VulkanFeatures m_features;              /**< Feature details. */
    VkInstance m_instance;                  /**< Vulkan instance handle. */
    VulkanInstanceFunctions m_functions;    /**< Instance function pointer table. */
    VulkanSurface *m_surface;               /**< Surface for the main window. */
    VulkanDevice *m_device;                 /**< Main logical device. */
    VulkanQueue *m_queue;                   /**< Device queue. */
    VulkanCommandPool *m_commandPool;       /**< Command buffer pool. */
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
    std::list<VulkanFrame *> m_frames;

    /** Primary command buffer for the current frame. */
    VulkanCommandBuffer *m_primaryCmdBuf;
};

/** Base class for a Vulkan child object. */
class VulkanObject {
public:
    /** @return             Manager that owns the object. */
    VulkanGPUManager *manager() const { return m_manager; }
protected:
    /** Initialise the object.
     * @param manager       Manager that owns the object. */
    explicit VulkanObject(VulkanGPUManager *manager) :
        m_manager(manager)
    {}
private:
    VulkanGPUManager *m_manager;            /**< Manager that owns the object. */
};

/** Base class for a Vulkan object that owns a handle. */
template <typename T>
class VulkanHandle : public VulkanObject {
public:
    /** @return             Handle to the object. */
    T handle() const { return m_handle; }
protected:
    /** Initialise the object.
     * @param manager       Manager that owns the object. */
    explicit VulkanHandle(VulkanGPUManager *manager) :
        VulkanObject(manager),
        m_handle(VK_NULL_HANDLE)
    {}

    T m_handle;                             /**< Handle to the object. */
};
