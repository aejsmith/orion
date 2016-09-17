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

#include "manager.h"
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
            createInfo.arrayLayers = CubeFace::kNumFaces;
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
    manager()->memoryManager()->freeResource(m_allocation);
}

/** Calculate the size of a mip level.
 * @param mip           Mip level.
 * @param width         Base level width, set to mip level width.
 * @param height        Base level height, set to mip level height. */
static inline void calcMipDimensions(unsigned mip, int &width, int &height) {
    for (unsigned i = 0; i < mip; i++) {
        if (width > 1)
            width >>= 1;
        if (height > 1)
            height >>= 1;
    }
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
    calcMipDimensions(mip, mipWidth, mipHeight);

    check(area.width <= mipWidth && area.height <= mipHeight);

    bool isWholeSubresource = area.width == mipWidth && area.height == mipHeight;

    auto memoryManager = manager()->memoryManager();
    auto stagingCmdBuf = memoryManager->getStagingCmdBuf();

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

    /* Ensure that the texture and its memory are kept alive until the update
     * is complete. */
    stagingCmdBuf->addObjectRef(this);
    stagingCmdBuf->addMemoryRef(m_allocation);
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
    check(!PixelFormat::isDepth(m_format));
    check(m_type != kTexture3D);

    unsigned numLayers;
    switch (m_type) {
        case kTexture2DArray:
            numLayers = m_depth;
            break;
        case kTextureCube:
            numLayers = CubeFace::kNumFaces;
            break;
        default:
            numLayers = 1;
            break;
    }

    /* We have to manually generate each mip by blitting from the base mip. */
    std::vector<VkImageBlit> imageBlits;
    imageBlits.reserve(m_mips - 1);

    int32_t mipWidth = m_width;
    int32_t mipHeight = m_height;

    for (unsigned mip = 1; mip < m_mips; mip++) {
        if (mipWidth > 1)
            mipWidth >>= 1;
        if (mipHeight > 1)
            mipHeight >>= 1;

        imageBlits.emplace_back();
        VkImageBlit &imageBlit = imageBlits.back();

        /* Copy the base mip level for all layers to the current mip level. */
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.mipLevel = 0;
        imageBlit.srcSubresource.baseArrayLayer = 0;
        imageBlit.srcSubresource.layerCount = numLayers;
        imageBlit.srcOffsets[0] = { 0, 0, 0 };
        imageBlit.srcOffsets[1] = { static_cast<int32_t>(m_width), static_cast<int32_t>(m_height), 1 };
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.mipLevel = mip;
        imageBlit.dstSubresource.baseArrayLayer = 0;
        imageBlit.dstSubresource.layerCount = numLayers;
        imageBlit.dstOffsets[0] = { 0, 0, 0 };
        imageBlit.dstOffsets[1] = { mipWidth, mipHeight, 1 };
    }

    VulkanCommandBuffer *stagingCmdBuf = manager()->memoryManager()->getStagingCmdBuf();

    /* Transition the base level to the transfer source layout. */
    VkImageSubresourceRange srcSubresource = {};
    srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    srcSubresource.baseMipLevel = 0;
    srcSubresource.levelCount = 1;
    srcSubresource.baseArrayLayer = 0;
    srcSubresource.layerCount = numLayers;
    VulkanUtil::setImageLayout(
        stagingCmdBuf,
        m_handle,
        srcSubresource,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    /* Transition the other levels to the transfer destination layout. Don't
     * care about their existing content. */
    VkImageSubresourceRange dstSubresource = {};
    dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    dstSubresource.baseMipLevel = 1;
    dstSubresource.levelCount = m_mips - 1;
    dstSubresource.baseArrayLayer = 0;
    dstSubresource.layerCount = numLayers;
    VulkanUtil::setImageLayout(
        stagingCmdBuf,
        m_handle,
        dstSubresource,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    /* Perform the blits. */
    vkCmdBlitImage(
        stagingCmdBuf->handle(),
        m_handle,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        m_handle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        imageBlits.size(),
        &imageBlits[0],
        VK_FILTER_LINEAR);

    /* Transition the whole image back to shader read only. */
    VulkanUtil::setImageLayout(
        stagingCmdBuf,
        m_handle,
        srcSubresource,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VulkanUtil::setImageLayout(
        stagingCmdBuf,
        m_handle,
        dstSubresource,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    stagingCmdBuf->addObjectRef(this);
    stagingCmdBuf->addMemoryRef(m_allocation);
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

/** Copy pixels from one texture to another.
 * @param source        Source texture image reference.
 * @param dest          Destination texture image reference.
 * @param sourcePos     Position in source texture to copy from.
 * @param destPos       Position in destination texture to copy to.
 * @param size          Size of area to copy. */
void VulkanGPUManager::blit(
    const GPUTextureImageRef &source,
    const GPUTextureImageRef &dest,
    glm::ivec2 sourcePos,
    glm::ivec2 destPos,
    glm::ivec2 size)
{
    // TODO: If formats are the same, we can use CopyImage.

    // FIXME: main window
    check(source && dest);

    /* If copying a depth texture, both formats must match. */
    bool isDepth = source && PixelFormat::isDepth(source.texture->format());
    check(isDepth == (dest && PixelFormat::isDepth(dest.texture->format())));
    check(!isDepth || source.texture->format() == dest.texture->format());

    VkImageBlit imageBlit = {};

    if (isDepth) {
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (PixelFormat::isDepthStencil(source.texture->format())) {
            imageBlit.srcSubresource.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            imageBlit.dstSubresource.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    imageBlit.srcSubresource.mipLevel = source.mip;
    imageBlit.srcSubresource.baseArrayLayer = source.layer;
    imageBlit.srcSubresource.layerCount = 1;
    imageBlit.srcOffsets[0] = { sourcePos.x, sourcePos.y, 0 };
    imageBlit.srcOffsets[1] = { sourcePos.x + size.x, sourcePos.y + size.y, 1 };
    imageBlit.dstSubresource.mipLevel = dest.mip;
    imageBlit.dstSubresource.baseArrayLayer = dest.layer;
    imageBlit.dstSubresource.layerCount = 1;
    imageBlit.dstOffsets[0] = { destPos.x, destPos.y, 0 };
    imageBlit.dstOffsets[1] = { destPos.x + size.x, destPos.y + size.y, 1 };

    /* Determine if we're overwriting the whole destination, in which case we
     * can ignore the existing image content. */
    int mipWidth = dest.texture->width();
    int mipHeight = dest.texture->height();
    calcMipDimensions(dest.mip, mipWidth, mipHeight);
    bool isWholeDestSubresource =
        destPos.x == 0 && destPos.y == 0 &&
        size.x == mipWidth && size.y == mipHeight;

    VulkanTexture *vkSource = static_cast<VulkanTexture *>(source.texture);
    VulkanTexture *vkDest = static_cast<VulkanTexture *>(dest.texture);

    VulkanCommandBuffer *primaryCmdBuf = currentFrame().primaryCmdBuf;

    /* Transition the source subresource to the transfer source layout. */
    VkImageSubresourceRange srcSubresource = {};
    srcSubresource.aspectMask = imageBlit.srcSubresource.aspectMask;
    srcSubresource.baseMipLevel = source.mip;
    srcSubresource.levelCount = 1;
    srcSubresource.baseArrayLayer = source.layer;
    srcSubresource.layerCount = 1;
    VulkanUtil::setImageLayout(
        primaryCmdBuf,
        vkSource->handle(),
        srcSubresource,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    /* Transition the destination subresource to the transfer destination layout. */
    VkImageSubresourceRange dstSubresource = {};
    dstSubresource.aspectMask = imageBlit.dstSubresource.aspectMask;
    dstSubresource.baseMipLevel = dest.mip;
    dstSubresource.levelCount = 1;
    dstSubresource.baseArrayLayer = dest.layer;
    dstSubresource.layerCount = 1;
    VulkanUtil::setImageLayout(
        primaryCmdBuf,
        vkDest->handle(),
        dstSubresource,
        (isWholeDestSubresource) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    /* Perform the blit. */
    vkCmdBlitImage(
        primaryCmdBuf->handle(),
        vkSource->handle(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        vkDest->handle(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &imageBlit,
        VK_FILTER_NEAREST);

    /* Transition the images back to shader read only. */
    VulkanUtil::setImageLayout(
        primaryCmdBuf,
        vkSource->handle(),
        srcSubresource,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VulkanUtil::setImageLayout(
        primaryCmdBuf,
        vkDest->handle(),
        dstSubresource,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    primaryCmdBuf->addObjectRef(vkSource);
    primaryCmdBuf->addObjectRef(vkDest);
    primaryCmdBuf->addMemoryRef(vkSource->allocation());
    primaryCmdBuf->addMemoryRef(vkDest->allocation());
}
