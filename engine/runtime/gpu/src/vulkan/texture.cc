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
 * @brief               Vulkan texture implementation.
 */

#include "manager.h"
#include "texture.h"

#include "gpu/utility.h"

/** Initialize a new texture.
 * @param manager       Manager that owns this texture.
 * @param desc          Texture descriptor. */
VulkanTexture::VulkanTexture(VulkanGPUManager *manager, const GPUTextureDesc &desc) :
    GPUTexture(desc),
    VulkanHandle(manager)
{
    VkDevice device = manager->device()->handle();

    VkImageCreateInfo createInfo = {};
    createInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.format        = manager->features().formats[desc.format].format;
    createInfo.extent.width  = desc.width;
    createInfo.extent.height = desc.height;
    createInfo.mipLevels     = m_mips;
    createInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    createInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.usage         = VK_IMAGE_USAGE_SAMPLED_BIT |
                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (desc.flags & kRenderTarget) {
        if (PixelFormat::isDepth(desc.format)) {
            createInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            createInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    } else {
        /* Allow creating views with differing formats. */
        createInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }

    switch (desc.type) {
        case kTexture2D:
            createInfo.imageType    = VK_IMAGE_TYPE_2D;
            createInfo.extent.depth = 1;
            createInfo.arrayLayers  = 1;
            break;
        case kTexture2DArray:
            createInfo.imageType    = VK_IMAGE_TYPE_2D;
            createInfo.extent.depth = 1;
            createInfo.arrayLayers  = desc.depth;
            break;
        case kTextureCube:
            createInfo.imageType    = VK_IMAGE_TYPE_2D;
            createInfo.extent.depth = 1;
            createInfo.arrayLayers  = CubeFace::kNumFaces;
            createInfo.flags       |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            break;
        case kTexture3D:
            createInfo.imageType    = VK_IMAGE_TYPE_3D;
            createInfo.extent.depth = desc.depth;
            createInfo.arrayLayers  = 1;
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

    /* Set the initial image layout. */
    VkImageSubresourceRange subresources;
    subresources.aspectMask     = VulkanUtil::aspectMaskForFormat(m_format);
    subresources.baseMipLevel   = 0;
    subresources.levelCount     = m_mips;
    subresources.baseArrayLayer = 0;
    subresources.layerCount     = createInfo.arrayLayers;
    VulkanUtil::setImageLayout(manager->memoryManager()->getStagingCmdBuf(),
                               m_handle,
                               subresources,
                               VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    /* Create a resource view. */
    // TODO: Only have depth for depth/stencil formats for now, because we can
    // only have one aspect set when using an image in a descriptor set.
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image                           = m_handle;
    viewCreateInfo.format                          = manager->features().formats[desc.format].format;
    viewCreateInfo.subresourceRange.aspectMask     = (PixelFormat::isDepth(desc.format))
                                                         ? VK_IMAGE_ASPECT_DEPTH_BIT
                                                         : VK_IMAGE_ASPECT_COLOR_BIT;
    viewCreateInfo.subresourceRange.baseMipLevel   = 0;
    viewCreateInfo.subresourceRange.levelCount     = m_mips;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount     = createInfo.arrayLayers;
    viewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_R;
    viewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_G;
    viewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_B;
    viewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_A;

    switch (desc.type) {
        case kTexture2D:
            viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            break;

        case kTexture2DArray:
            viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            break;

        case kTextureCube:
            viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            break;

        case kTexture3D:
            viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;

        default:
            unreachable();

    }

    checkVk(vkCreateImageView(device, &viewCreateInfo, nullptr, &m_resourceView));
}

/** Initialize a new texture view.
 * @param manager       Manager that owns this texture.
 * @param desc          Descriptor for the view. */
VulkanTexture::VulkanTexture(VulkanGPUManager *manager, const GPUTextureViewDesc &desc) :
    GPUTexture   (desc),
    VulkanHandle (manager)
{
    auto source = static_cast<VulkanTexture *>(m_source.get());

    m_handle = source->handle();

    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image                           = m_handle;
    viewCreateInfo.format                          = manager->features().formats[m_format].format;
    viewCreateInfo.subresourceRange.aspectMask     = (PixelFormat::isDepth(m_format))
                                                         ? VK_IMAGE_ASPECT_DEPTH_BIT
                                                         : VK_IMAGE_ASPECT_COLOR_BIT;
    viewCreateInfo.subresourceRange.baseMipLevel   = m_baseMip;
    viewCreateInfo.subresourceRange.levelCount     = m_mips;
    viewCreateInfo.subresourceRange.baseArrayLayer = m_baseLayer;
    viewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_R;
    viewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_G;
    viewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_B;
    viewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_A;

    switch (desc.type) {
        case kTexture2D:
            viewCreateInfo.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
            viewCreateInfo.subresourceRange.layerCount = 1;
            break;

        case kTexture2DArray:
            viewCreateInfo.viewType                    = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            viewCreateInfo.subresourceRange.layerCount = m_depth;
            break;

        case kTextureCube:
            viewCreateInfo.viewType                    = VK_IMAGE_VIEW_TYPE_CUBE;
            viewCreateInfo.subresourceRange.layerCount = CubeFace::kNumFaces;
            break;

        case kTexture3D:
            viewCreateInfo.viewType                    = VK_IMAGE_VIEW_TYPE_3D;
            viewCreateInfo.subresourceRange.layerCount = 1;
            break;

    }

    checkVk(vkCreateImageView(manager->device()->handle(),
                              &viewCreateInfo,
                              nullptr,
                              &m_resourceView));
}

/** Destroy the texture. */
VulkanTexture::~VulkanTexture() {
    manager()->invalidateFramebuffers(this);

    vkDestroyImageView(manager()->device()->handle(), m_resourceView, nullptr);

    if (!isView()) {
        vkDestroyImage(manager()->device()->handle(), m_handle, nullptr);
        manager()->memoryManager()->freeResource(m_allocation);
    }
}

/** Update 2D texture area.
 * @param area          Area to update (2D rectangle).
 * @param data          Data to update with.
 * @param layer         Array layer/cube face.
 * @param mip           Mipmap level. */
void VulkanTexture::update(const IntRect &area, const void *data, unsigned mip, unsigned layer) {
    check(!isView());
    check(m_type == kTexture2D || m_type == kTexture2DArray || m_type == kTextureCube);
    check(mip < m_mips);
    check(layer < m_depth);
    check(area.width >= 0 && area.height >= 0);
    check(!PixelFormat::isDepth(m_format));

    auto areaWidth = static_cast<uint32_t>(area.width);
    auto areaHeight = static_cast<uint32_t>(area.height);

    if (!areaWidth || !areaHeight)
        return;

    /* Get mip level size. */
    uint32_t mipWidth = m_width;
    uint32_t mipHeight = m_height;
    GPUUtil::calcMipDimensions(mip, mipWidth, mipHeight);

    check(areaWidth <= mipWidth && areaHeight <= mipHeight);

    bool isWholeSubresource = areaWidth == mipWidth && areaHeight == mipHeight;

    auto memoryManager = manager()->memoryManager();
    auto stagingCmdBuf = memoryManager->getStagingCmdBuf();

    /* Allocate a staging buffer large enough and copy to it. */
    VkDeviceSize dataSize = areaWidth * areaHeight * PixelFormat::bytesPerPixel(m_format);
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
    VulkanUtil::setImageLayout(stagingCmdBuf,
                               m_handle,
                               subresources,
                               (isWholeSubresource)
                                   ? VK_IMAGE_LAYOUT_UNDEFINED
                                   : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    /* Copy the image data. */
    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = mip;
    region.imageSubresource.baseArrayLayer = layer;
    region.imageSubresource.layerCount = 1;
    region.imageOffset.x = area.x;
    region.imageOffset.y = area.y;
    region.imageExtent.width = areaWidth;
    region.imageExtent.height = areaHeight;
    region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(stagingCmdBuf->handle(),
                           staging->buffer(),
                           m_handle,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);

    /* Transition back to shader read only. */
    VulkanUtil::setImageLayout(stagingCmdBuf,
                               m_handle,
                               subresources,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    /* Ensure that the texture is kept alive until the update is complete. Our
     * memory allocation lifetime is tied to our own lifetime so there's no
     * need to reference them separately. */
    stagingCmdBuf->addReference(this);
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
    check(!isView());
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

    VulkanCommandBuffer *cmdBuf = manager()->currentFrame().primaryCmdBuf;

    /* Transition the base level to the transfer source layout. */
    VkImageSubresourceRange srcSubresource = {};
    srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    srcSubresource.baseMipLevel = 0;
    srcSubresource.levelCount = 1;
    srcSubresource.baseArrayLayer = 0;
    srcSubresource.layerCount = numLayers;
    VulkanUtil::setImageLayout(cmdBuf,
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
    VulkanUtil::setImageLayout(cmdBuf,
                               m_handle,
                               dstSubresource,
                               VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    /* Perform the blits. */
    vkCmdBlitImage(cmdBuf->handle(),
                   m_handle,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   m_handle,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   imageBlits.size(), &imageBlits[0],
                   VK_FILTER_LINEAR);

    /* Transition the whole image back to shader read only. */
    VulkanUtil::setImageLayout(cmdBuf,
                               m_handle,
                               srcSubresource,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VulkanUtil::setImageLayout(cmdBuf,
                               m_handle,
                               dstSubresource,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    cmdBuf->addReference(this);
}

/** Create a texture.
 * @param desc          Descriptor containing texture parameters.
 * @return              Pointer to created texture. */
GPUTexturePtr VulkanGPUManager::createTexture(const GPUTextureDesc &desc) {
    return new VulkanTexture(this, desc);
}

/** Create a texture view.
 * @param desc          Descriptor for the view.
 * @return              Pointer to created texture view. */
GPUTexturePtr VulkanGPUManager::createTextureView(const GPUTextureViewDesc &desc) {
    return new VulkanTexture(this, desc);
}

/** Initialise the sampler state object.
 * @param manager       Manager that owns the object.
 * @param desc          Descriptor for sampler state. */
VulkanSamplerState::VulkanSamplerState(VulkanGPUManager *manager, const GPUSamplerStateDesc &desc) :
    GPUSamplerState (desc),
    VulkanHandle    (manager)
{
    VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.maxAnisotropy = 1.0f;

    switch (m_desc.filterMode) {
        case SamplerFilterMode::kBilinear:
            createInfo.magFilter        = VK_FILTER_LINEAR;
            createInfo.minFilter        = VK_FILTER_LINEAR;
            createInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case SamplerFilterMode::kTrilinear:
            createInfo.magFilter        = VK_FILTER_LINEAR;
            createInfo.minFilter        = VK_FILTER_LINEAR;
            createInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case SamplerFilterMode::kAnisotropic:
            createInfo.magFilter        = VK_FILTER_LINEAR;
            createInfo.minFilter        = VK_FILTER_LINEAR;
            createInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            createInfo.anisotropyEnable = true;

            /* TODO: global default if set to 0, see GL note about hashing. */
            createInfo.maxAnisotropy    = glm::clamp(
                static_cast<float>(m_desc.maxAnisotropy),
                1.0f,
                manager->device()->limits().maxSamplerAnisotropy);
            break;
        default:
            createInfo.magFilter        = VK_FILTER_NEAREST;
            createInfo.minFilter        = VK_FILTER_NEAREST;
            createInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_NEAREST;
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

    createInfo.addressModeU  = convertAddressMode(desc.addressU);
    createInfo.addressModeV  = convertAddressMode(desc.addressV);
    createInfo.addressModeW  = convertAddressMode(desc.addressW);

    createInfo.compareEnable = desc.compareEnable;
    createInfo.compareOp     = VulkanUtil::convertComparisonFunc(desc.compareFunc);

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
void VulkanGPUManager::blit(const GPUTextureImageRef &source,
                            const GPUTextureImageRef &dest,
                            glm::ivec2 sourcePos,
                            glm::ivec2 destPos,
                            glm::ivec2 size)
{
    check(source);
    check(dest);

    auto sourceTexture = static_cast<VulkanTexture *>(source.texture);
    auto destTexture   = static_cast<VulkanTexture *>(dest.texture);

    const bool formatsMatch = sourceTexture->format() == destTexture->format();

    /* If copying a depth texture, both formats must match. */
    const bool isDepth = PixelFormat::isDepth(sourceTexture->format());
    check(isDepth == PixelFormat::isDepth(destTexture->format()));
    check(!isDepth || formatsMatch);

    /* Must have matching aspects between formats. */
    const uint32_t aspectMask = VulkanUtil::aspectMaskForFormat(sourceTexture->format());
    check(aspectMask == VulkanUtil::aspectMaskForFormat(destTexture->format()));

    /* Determine if we're overwriting the whole destination, in which case we
     * can ignore the existing image content. */
    uint32_t mipWidth  =  dest.texture->width();
    uint32_t mipHeight =  dest.texture->height();
    GPUUtil::calcMipDimensions(dest.mip, mipWidth, mipHeight);
    const bool isWholeDestSubresource = destPos.x == 0 &&
                                        destPos.y == 0 &&
                                        static_cast<uint32_t>(size.x) == mipWidth &&
                                        static_cast<uint32_t>(size.y) == mipHeight;

    /* Reference the images. */
    VulkanCommandBuffer *primaryCmdBuf = currentFrame().primaryCmdBuf;
    primaryCmdBuf->addReference(sourceTexture);
    primaryCmdBuf->addReference(destTexture);

    /* Transition the source subresource to the transfer source layout. */
    VkImageSubresourceRange srcSubresource = {};
    srcSubresource.aspectMask     = aspectMask;
    srcSubresource.baseMipLevel   = source.mip;
    srcSubresource.levelCount     = 1;
    srcSubresource.baseArrayLayer = source.layer;
    srcSubresource.layerCount     = 1;

    VulkanUtil::setImageLayout(primaryCmdBuf,
                               sourceTexture->handle(),
                               srcSubresource,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    /* Transition the destination subresource to the transfer destination layout. */
    VkImageSubresourceRange dstSubresource = {};
    dstSubresource.aspectMask     = aspectMask;
    dstSubresource.baseMipLevel   = dest.mip;
    dstSubresource.levelCount     = 1;
    dstSubresource.baseArrayLayer = dest.layer;
    dstSubresource.layerCount     = 1;

    VulkanUtil::setImageLayout(primaryCmdBuf,
                               destTexture->handle(),
                               dstSubresource,
                               (isWholeDestSubresource)
                                   ? VK_IMAGE_LAYOUT_UNDEFINED
                                   : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    /* Use a copy if the formats match. TODO: Also do for compatible formats. */
    if (formatsMatch) {
        VkImageCopy imageCopy = {};
        imageCopy.srcSubresource.aspectMask     = aspectMask;
        imageCopy.srcSubresource.mipLevel       = source.mip;
        imageCopy.srcSubresource.baseArrayLayer = source.layer;
        imageCopy.srcSubresource.layerCount     = 1;
        imageCopy.srcOffset                     = { sourcePos.x, sourcePos.y, 0 };
        imageCopy.dstSubresource.aspectMask     = aspectMask;
        imageCopy.dstSubresource.mipLevel       = dest.mip;
        imageCopy.dstSubresource.baseArrayLayer = dest.layer;
        imageCopy.dstSubresource.layerCount     = 1;
        imageCopy.dstOffset                     = { destPos.x, destPos.y, 0 };
        imageCopy.extent                        = { static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), 1 };

        vkCmdCopyImage(primaryCmdBuf->handle(),
                       sourceTexture->handle(),
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       destTexture->handle(),
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &imageCopy);
    } else {
        VkImageBlit imageBlit = {};
        imageBlit.srcSubresource.aspectMask     = aspectMask;
        imageBlit.srcSubresource.mipLevel       = source.mip;
        imageBlit.srcSubresource.baseArrayLayer = source.layer;
        imageBlit.srcSubresource.layerCount     = 1;
        imageBlit.srcOffsets[0]                 = { sourcePos.x, sourcePos.y, 0 };
        imageBlit.srcOffsets[1]                 = { sourcePos.x + size.x, sourcePos.y + size.y, 1 };
        imageBlit.dstSubresource.aspectMask     = aspectMask;
        imageBlit.dstSubresource.mipLevel       = dest.mip;
        imageBlit.dstSubresource.baseArrayLayer = dest.layer;
        imageBlit.dstSubresource.layerCount     = 1;
        imageBlit.dstOffsets[0]                 = { destPos.x, destPos.y, 0 };
        imageBlit.dstOffsets[1]                 = { destPos.x + size.x, destPos.y + size.y, 1 };

        vkCmdBlitImage(primaryCmdBuf->handle(),
                       sourceTexture->handle(),
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       destTexture->handle(),
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &imageBlit,
                       VK_FILTER_NEAREST);
    }

    /* Transition the images back to shader read only. */
    VulkanUtil::setImageLayout(primaryCmdBuf,
                               sourceTexture->handle(),
                               srcSubresource,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VulkanUtil::setImageLayout(primaryCmdBuf,
                               destTexture->handle(),
                               dstSubresource,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
