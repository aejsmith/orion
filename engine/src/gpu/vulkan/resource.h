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
 * @brief               Vulkan resource set implementation.
 */

#pragma once

#include "vulkan.h"

class VulkanCommandBuffer;

/** Vulkan resource set layout implementation. */
class VulkanResourceSetLayout :
    public GPUResourceSetLayout,
    public VulkanHandle<VkDescriptorSetLayout> {
public:
    VulkanResourceSetLayout(VulkanGPUManager *manager, GPUResourceSetLayoutDesc &&desc);
    ~VulkanResourceSetLayout();
};

/** Class managing a Vulkan descriptor pool. */
class VulkanDescriptorPool : public VulkanHandle<VkDescriptorPool> {
public:
    explicit VulkanDescriptorPool(VulkanGPUManager *manager);
    ~VulkanDescriptorPool();
};

/**
 * Vulkan resource set implementation.
 *
 * Management of resource sets is a bit more complex than it might seem on the
 * surface. We cannot modify a Vulkan descriptor set while it is in use on the
 * GPU. However, after we've submitted a frame and move on to the next, the
 * engine might ask us to update a resource set even if the previous frame
 * hasn't actually finished, because as far as the engine is concerned, it has.
 * Therefore we cannot just have an engine-level resource set correspond
 * directly to a single Vulkan descriptor set.
 *
 * Instead, we maintain multiple descriptor sets per resource set object. When
 * the engine asks us to modify a resource set (via updateSlot()) we just flag
 * the slot as dirty. Once we get to a draw call with a dirty resource set
 * bound, we check if its current descriptor set still might be in use (via it's
 * reference count). If it is, we create a new descriptor set and apply the
 * updates to that and use it for rendering, and release the reference held to
 * the old one so that it will be freed when the frame it was used in completes.
 */
class VulkanResourceSet : public GPUResourceSet, public VulkanObject {
public:
    VulkanResourceSet(VulkanGPUManager *manager, GPUResourceSetLayout *layout);
    ~VulkanResourceSet();

    VkDescriptorSet prepareForDraw(VulkanCommandBuffer *cmdBuf);
protected:
    void updateSlot(size_t index) override;
private:
    /** Descriptor set. */
    class DescriptorSet : public Refcounted, public VulkanHandle<VkDescriptorSet> {
    public:
        DescriptorSet(VulkanGPUManager *manager, const VulkanResourceSetLayout *layout);
    protected:
        ~DescriptorSet();
    };

    /** Current descriptor set. */
    ReferencePtr<DescriptorSet> m_current;

    /** Currently dirty slots. */
    std::vector<bool> m_dirtySlots;
};
