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
 * @brief               Vulkan texture implementation.
 */

#include "command_buffer.h"
#include "texture.h"

/** Initialize a new texture.
 * @param manager       Manager that owns this texture.
 * @param desc          Texture descriptor. */
VulkanTexture::VulkanTexture(VulkanGPUManager *manager, const GPUTextureDesc &desc) :
    GPUTexture(desc),
    VulkanHandle(manager),
    m_new(true)
{
    VkDevice device = manager->device()->handle();

    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.format = manager->features().formats[desc.format].format;
    createInfo.extent.width = desc.width;
    createInfo.extent.height = desc.height;
    createInfo.mipLevels = m_mips;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.usage =
        VK_IMAGE_USAGE_SAMPLED_BIT |
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (desc.flags & kRenderTarget) {
        if (PixelFormat::isDepth(desc.format)) {
            createInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            createInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }

    switch (desc.type) {
        case kTexture2D:
            createInfo.imageType = VK_IMAGE_TYPE_2D;
            createInfo.extent.depth = 1;
            createInfo.arrayLayers = 1;
            break;
        case kTexture2DArray:
            createInfo.imageType = VK_IMAGE_TYPE_2D;
            createInfo.extent.depth = 1;
            createInfo.arrayLayers = desc.depth;
            break;
        case kTextureCube:
            createInfo.imageType = VK_IMAGE_TYPE_2D;
            createInfo.extent.depth = 1;
            createInfo.arrayLayers = 6;
            createInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            break;
        case kTexture3D:
            createInfo.imageType = VK_IMAGE_TYPE_3D;
            createInfo.extent.depth = desc.depth;
            createInfo.arrayLayers = 1;
            break;
        default:
            unreachable();
    }

    checkVk(vkCreateImage(device, &createInfo, nullptr, &m_handle));

    /* Get the memory requirements. */
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device, m_handle, &requirements);

    /* Allocate memory for the image. */
    m_allocation = manager->memoryManager()->allocateImage(requirements);

    /* Bind the memory to the image. */
    checkVk(vkBindImageMemory(device, m_handle, m_allocation->memory(), m_allocation->offset()));
}

/** Initialize a new texture view.
 * @param manager       Manager that owns this texture.
 * @param image         Image to create the view for.
 * @return              Pointer to created texture view. */
VulkanTexture::VulkanTexture(VulkanGPUManager *manager, const GPUTextureImageRef &image) :
    GPUTexture(image),
    VulkanHandle(manager)
{
    // mutable format bit
    fatal("TODO: Vulkan texture view");
}

/** Destroy the texture. */
VulkanTexture::~VulkanTexture() {
    vkDestroyImage(manager()->device()->handle(), m_handle, nullptr);
    manager()->memoryManager()->freeImage(m_allocation);
}

/** Update 2D texture area.
 * @param area          Area to update (2D rectangle).
 * @param data          Data to update with.
 * @param layer         Array layer/cube face.
 * @param mip           Mipmap level. */
void VulkanTexture::update(const IntRect &area, const void *data, unsigned mip, unsigned layer) {
    check(m_type == kTexture2D || m_type == kTexture2DArray || m_type == kTextureCube);
    check(mip < m_mips);
    check(layer < m_depth);
    check(area.width >= 0 && area.height >= 0);
    check(!PixelFormat::isDepth(m_format));

    if (!area.width || !area.height)
        return;

    /* Get mip level size. */
    int mipWidth = m_width;
    int mipHeight = m_height;
    for (unsigned i = 0; i < mip; i++) {
        if (mipWidth > 1)
            mipWidth >>= 1;
        if (mipHeight > 1)
            mipHeight >>= 1;
    }

    check(area.width <= mipWidth && area.height <= mipHeight);

    bool isWholeSubresource = area.width == mipWidth && area.height == mipHeight;

    auto memoryManager = manager()->memoryManager();
    auto stagingCmdBuf = memoryManager->getStagingCommandBuffer();

    /* Allocate a staging buffer large enough and copy to it. */
    VkDeviceSize dataSize = area.width * area.height * PixelFormat::bytesPerPixel(m_format);
    VulkanMemoryManager::StagingMemory *staging = memoryManager->allocateStagingMemory(dataSize);
    memcpy(staging->map(), data, dataSize);

    /* Transition to the transfer destination format. If we are replacing the
     * whole subresource data, we can use undefined as the source layout to
     * indicate that we don't care about it. */
    VkImageSubresourceRange subresources = {};
    subresources.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresources.baseMipLevel = mip;
    subresources.levelCount = 1;
    subresources.baseArrayLayer = layer;
    subresources.layerCount = 1;
    VulkanUtil::setImageLayout(
        stagingCmdBuf,
        m_handle,
        subresources,
        (isWholeSubresource) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    /* Copy the image data. */
    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = mip;
    region.imageSubresource.baseArrayLayer = layer;
    region.imageSubresource.layerCount = 1;
    region.imageOffset.x = area.x;
    region.imageOffset.y = area.y;
    region.imageExtent.width = area.width;
    region.imageExtent.height = area.height;
    vkCmdCopyBufferToImage(
        stagingCmdBuf->handle(),
        staging->buffer(),
        m_handle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    /* Transition back to shader read only. */
    VulkanUtil::setImageLayout(
        stagingCmdBuf,
        m_handle,
        subresources,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

/** Update 3D texture area.
 * @param area          Area to update (3D box).
 * @param data          Data to update with.
 * @param mip           Mipmap level. */
void VulkanTexture::update(const IntBox &area, const void *data, unsigned mip) {
    fatal("TODO: 3D texture update");
}

/** Generate mipmap images. */
void VulkanTexture::generateMipmap() {
    check(m_flags & kAutoMipmap);

    fatal("TODO: mipmap generation");
}

/** Create a texture.
 * @param desc          Descriptor containing texture parameters.
 * @return              Pointer to created texture. */
GPUTexturePtr VulkanGPUManager::createTexture(const GPUTextureDesc &desc) {
    return new VulkanTexture(this, desc);
}

/** Create a texture view.
 * @param image         Image to create the view for.
 * @return              Pointer to created texture view. */
GPUTexturePtr VulkanGPUManager::createTextureView(const GPUTextureImageRef &image) {
    return new VulkanTexture(this, image);
}

/** Initialise the sampler state object.
 * @param manager       Manager that owns the object.
 * @param desc          Descriptor for sampler state. */
VulkanSamplerState::VulkanSamplerState(VulkanGPUManager *manager, const GPUSamplerStateDesc &desc) :
    GPUSamplerState(desc),
    VulkanHandle(manager)
{
    VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    switch (m_desc.filterMode) {
        case SamplerFilterMode::kBilinear:
            createInfo.magFilter = VK_FILTER_LINEAR;
            createInfo.minFilter = VK_FILTER_LINEAR;
            createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case SamplerFilterMode::kTrilinear:
            createInfo.magFilter = VK_FILTER_LINEAR;
            createInfo.minFilter = VK_FILTER_LINEAR;
            createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case SamplerFilterMode::kAnisotropic:
            createInfo.magFilter = VK_FILTER_LINEAR;
            createInfo.minFilter = VK_FILTER_LINEAR;
            createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            createInfo.anisotropyEnable = true;
            /* TODO: global default if set to 0, see GL note about hashing. */
            createInfo.maxAnisotropy = glm::clamp(
                static_cast<float>(m_desc.maxAnisotropy),
                1.0f,
                manager->device()->limits().maxSamplerAnisotropy);
            break;
        default:
            createInfo.magFilter = VK_FILTER_NEAREST;
            createInfo.minFilter = VK_FILTER_NEAREST;
            createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
    }

    auto convertAddressMode =
        [] (SamplerAddressMode mode) -> VkSamplerAddressMode {
            switch (mode) {
                case SamplerAddressMode::kClamp:
                    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                case SamplerAddressMode::kWrap:
                    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            }
        };

    createInfo.addressModeU = convertAddressMode(desc.addressU);
    createInfo.addressModeV = convertAddressMode(desc.addressV);
    createInfo.addressModeW = convertAddressMode(desc.addressW);

    checkVk(vkCreateSampler(manager->device()->handle(), &createInfo, nullptr, &m_handle));
}

/** Destroy the sampler state. */
VulkanSamplerState::~VulkanSamplerState() {
    vkDestroySampler(manager()->device()->handle(), m_handle, nullptr);
}

/** Create a sampler state object.
 * @param desc          Descriptor for sampler state.
 * @return              Pointer to created sampler state object. */
GPUSamplerStatePtr VulkanGPUManager::createSamplerState(const GPUSamplerStateDesc &desc) {
    return new VulkanSamplerState(this, desc);
}
