/*
 * Copyright (C) 2015 Alex Smith
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
 * @brief               Texture asset classes.
 *
 * TODO:
 *  - RenderTexture needs to keep its owning texture alive while it is in use.
 *    RenderLayer holds a RenderTarget pointer, which won't keep a reference to
 *    the texture. To solve this we could store a TextureBasePtr in
 *    RenderTexture, and have a way to be notified when the RenderTarget is no
 *    longer referred to by any layers, at which point we would drop the
 *    reference if the texture is not referred to anywhere else.
 */

#include "engine/debug_window.h"
#include "engine/texture.h"

#include "gpu/gpu_manager.h"

/**
 * Common texture implementation.
 */

/** Private constructor, does not actually create the texture. */
TextureBase::TextureBase() :
    m_filterMode  (SamplerFilterMode::kAnisotropic),
    m_anisotropy  (8),
    m_addressMode (SamplerAddressMode::kClamp)
{
    updateSamplerState();
}

/**
 * Set the texture filtering mode.
 *
 * Sets the texture filtering mode for this texture. By default, textures will
 * use the global texture filtering settings. Using this function will override
 * those settings for this particular texture.
 *
 * @param mode          Texture filtering mode to use.
 */
void TextureBase::setFilterMode(SamplerFilterMode mode) {
    // TODO: global filtering defaults.
    if (mode != m_filterMode) {
        m_filterMode = mode;
        updateSamplerState();
    }
}

/**
 * Set the anisotropy level.
 *
 * When the filtering mode is set to anisotropic, this sets the degree of
 * anisotropy used by the filtering process. Unless the filtering mode has been
 * overridden from the global defaults using setFilterMode(), this parameter is
 * ignored.
 *
 * @param anisotropy    Degree of anisotropy for anisotropic filtering.
 */
void TextureBase::setAnisotropy(unsigned anisotropy) {
    if (anisotropy != m_anisotropy) {
        m_anisotropy = anisotropy;
        updateSamplerState();
    }
}

/**
 * Set the texture addressing mode.
 *
 * This sets the method used for resolving texture coordinates outside the
 * (0, 1) range. When it is set to clamp, texture coordinates are clamped to
 * the (0, 1) range. When it is set to wrap, texture coordinates are wrapped
 * around, causing the texture to repeat.
 *
 * @param mode          Texture addressing mode to use.
 */
void TextureBase::setAddressMode(SamplerAddressMode mode) {
    if (mode != m_addressMode) {
        m_addressMode = mode;
        updateSamplerState();
    }
}

/** Recreate the texture sampler state. */
void TextureBase::updateSamplerState() {
    auto desc = GPUSamplerStateDesc().
        setFilterMode    (m_filterMode).
        setMaxAnisotropy (m_anisotropy).
        setAddressU      (m_addressMode).
        setAddressV      (m_addressMode).
        setAddressW      (m_addressMode);
    m_sampler = g_gpuManager->getSamplerState(desc);
}

/** Display details of the asset in the debug explorer. */
void TextureBase::explore() {
    if (m_gpu->type() == GPUTexture::kTexture3D || m_gpu->type() == GPUTexture::kTexture2DArray) {
        ImGui::Text("Size: %ux%ux%u", m_gpu->width(), m_gpu->height(), m_gpu->depth());
    } else {
        ImGui::Text("Size: %ux%u", m_gpu->width(), m_gpu->height());
    }

    unsigned layers;
    switch (m_gpu->type()) {
        case GPUTexture::kTexture2D:
            layers = 1;
            break;
        case GPUTexture::kTexture2DArray:
            layers = m_gpu->depth();
            break;
        case GPUTexture::kTextureCube:
            layers = CubeFace::kNumFaces;
            break;
        default:
            /* Unsupported for now. */
            return;
    }

    glm::vec2 texSize(m_gpu->width(), m_gpu->height());
    float scaleFactor = (texSize.x > texSize.y)
                            ? glm::min(128.0f, texSize.x) / texSize.x
                            : glm::min(128.0f, texSize.y) / texSize.y;
    glm::vec2 drawSize = texSize * scaleFactor;

    for (unsigned i = 0; i < layers; i++) {
        // FIXME: We're losing mipmapping because I was lazy when implementing
        // implementing texture views.
        GPUTexturePtr texture = (m_gpu->type() != GPUTexture::kTexture2D)
                                    ? g_gpuManager->createTextureView(GPUTextureImageRef(m_gpu, i, 0))
                                    : m_gpu;
        ImTextureID textureRef = DebugWindow::refTexture(texture);

        ImGui::Text("Image %u:", i);
        ImGui::SameLine(100);
        glm::vec2 texPos = ImGui::GetCursorScreenPos();
        ImGui::Image(textureRef,
                     drawSize,
                     ImVec2(0, 1), ImVec2(1, 0),
                     ImColor(255, 255, 255, 255),
                     ImColor(0, 0, 0, 0));

        /* If we scaled down the texture, add a popup to zoom over it. */
        if (scaleFactor < 1.0f && ImGui::IsItemHovered()) {
            float focusSize = std::min(512.0f, std::min(texSize.x, texSize.y));

            ImGui::BeginTooltip();

            glm::vec2 mousePos = ImGui::GetMousePos();
            glm::vec2 mouseRelPos = mousePos - texPos;
            glm::vec2 mouseTexPos(
                mouseRelPos.x / scaleFactor,
                texSize.y - (mouseRelPos.y / scaleFactor));
            float focusX = std::min(std::max(0.0f, mouseTexPos.x - focusSize * 0.5f),
                                    texSize.x - focusSize);
            float focusY = std::min(std::max(0.0f, mouseTexPos.y - focusSize * 0.5f),
                                    texSize.y - focusSize);

            ImGui::Text("Min: (%.2f, %.2f)", focusX, focusY);
            ImGui::Text("Max: (%.2f, %.2f)", focusX + focusSize, focusY + focusSize);

            ImGui::Image(textureRef,
                         ImVec2(focusSize, focusSize),
                         ImVec2(focusX / texSize.x, (focusY + focusSize) / texSize.y),
                         ImVec2((focusX + focusSize) / texSize.x, focusY / texSize.y),
                         ImColor(255, 255, 255, 255),
                         ImColor(0, 0, 0, 0));

            ImGui::EndTooltip();
        }
    }
}

/**
 * 2D texture implementation.
 */

/**
 * Create a 2D texture.
 *
 * Creates a new 2D texture. The default arguments give the texture a format of
 * PixelFormat::kR8G8B8A8, and a full mipmap pyramid which can be automatically
 * updated.
 *
 * @param width         Width of the texture.
 * @param height        Height of the texture.
 * @param format        Pixel format to use.
 * @param mips          Number of mip levels (0 for full pyramid).
 * @param flags         GPU texture creation flags.
 */
Texture2D::Texture2D(uint32_t width,
                     uint32_t height,
                     PixelFormat format,
                     unsigned mips,
                     uint32_t flags) :
    m_renderTexture (nullptr)
{
    auto desc = GPUTextureDesc().
        setType   (GPUTexture::kTexture2D).
        setWidth  (width).
        setHeight (height).
        setFormat (format).
        setMips   (mips).
        setFlags  (flags);

    m_gpu = g_gpuManager->createTexture(desc);

    /* Create a render texture if requested. */
    if (flags & GPUTexture::kRenderTarget)
        m_renderTexture = new RenderTexture(this, 0);
}

/** Destroy the texture. */
Texture2D::~Texture2D() {
    delete m_renderTexture;
}

/** Clear the entire texture contents to 0. */
void Texture2D::clear() {
    size_t size = m_gpu->width() * m_gpu->height() * PixelFormat::bytesPerPixel(m_gpu->format());
    std::unique_ptr<char[]> data(new char[size]);
    memset(data.get(), 0, size);
    update(data.get());
}

/**
 * Replace the entire texture content.
 *
 * Replaces the entire content of the top mip level of the texture with the
 * contents of the supplied buffer. The buffer must contain pixel data in the
 * same format as the texture, and it must equal the size of the texture.
 *
 * If updateMipmap is true (the default), the mipmap images of the texture
 * will be regenerated based on the new image content. Note that this will only
 * actually be done if the texture was created with the GPUTexture::kAutoMipmap
 * flag set.
 *
 * @param data          New texture data.
 * @param updateMipmap  Whether to update mipmap images (defaults to true).
 */
void Texture2D::update(const void *data, bool updateMipmap) {
    IntRect area(0, 0, m_gpu->width(), m_gpu->height());
    m_gpu->update(area, data);

    /* Regenerate mipmaps if requested. */
    if (updateMipmap && m_gpu->flags() & GPUTexture::kAutoMipmap)
        m_gpu->generateMipmap();
}

/**
 * Update an area of the texture.
 *
 * Updates an area of the top mip level of the texture. The buffer must contain
 * pixel data in the same format as the texture, and it must equal the area
 * size specified.
 *
 * If updateMipmap is true (the default), the mipmap images of the texture
 * will be regenerated based on the updated image content. Note that this will
 * only actually be done if the texture was created with the
 * GPUTexture::kAutoMipmap flag set.
 *
 * @param area          Area to update.
 * @param data          New texture data.
 * @param updateMipmap  Whether to update mipmap images (defaults to true).
 */
void Texture2D::update(const IntRect &area, const void *data, bool updateMipmap) {
    m_gpu->update(area, data);

    /* Regenerate mipmaps if requested. */
    if (updateMipmap && m_gpu->flags() & GPUTexture::kAutoMipmap)
        m_gpu->generateMipmap();
}

/**
 * Update a specific mip level of the texture.
 *
 * Updates an area of a specific mip level of the texture. The buffer must
 * contain pixel data in the same format as the texture, and it must equal the
 * area size specified. No mipmap regeneration will be performed.
 *
 * @param mip           Mip level to update.
 * @param area          Area to update.
 * @param data          New texture data.
 */
void Texture2D::update(unsigned mip, const IntRect &area, const void *data) {
    check(mip < mips());

    m_gpu->update(area, data, mip);
}

/**
 * Get the render texture for the texture.
 *
 * Gets the render texture referring to this texture. The texture must have been
 * created with the GPUTexture::kRenderTarget flag set.
 *
 * @return              Render texture referring to the texture.
 */
RenderTexture *Texture2D::renderTexture() {
    check(m_renderTexture);
    return m_renderTexture;
}

/**
 * Cube texture implementation.
 */

/**
 * Create a cube texture.
 *
 * Creates a new cube texture. The default arguments give the texture a format
 * of PixelFormat::kR8G8B8A8, and a full mipmap pyramid which can be
 * automatically updated.
 *
 * @param size          Size of the texture.
 * @param format        Pixel format to use.
 * @param mips          Number of mip levels (0 for full pyramid).
 * @param flags         GPU texture creation flags.
 */
TextureCube::TextureCube(uint32_t size, PixelFormat format, unsigned mips, uint32_t flags) {
    auto desc = GPUTextureDesc().
        setType   (GPUTexture::kTextureCube).
        setWidth  (size).
        setHeight (size).
        setFormat (format).
        setMips   (mips).
        setFlags  (flags);

    m_gpu = g_gpuManager->createTexture(desc);
}

/** Clear the entire texture contents to 0. */
void TextureCube::clear() {
    size_t size = this->size() * 2 * PixelFormat::bytesPerPixel(m_gpu->format());
    std::unique_ptr<char[]> data(new char[size]);
    memset(data.get(), 0, size);

    for (unsigned i = 0; i < CubeFace::kNumFaces; i++)
        update(i, data.get());
}

/**
 * Replace the entire texture content.
 *
 * Replaces the entire content of the top mip level of one of the faces of the
 * texture with the contents of the supplied buffer. The buffer must contain
 * pixel data in the same format as the texture, and it must equal the size of
 * the texture.
 *
 * If updateMipmap is true (the default), the mipmap images of the face will be
 * regenerated based on the new image content. Note that this will only actually
 * be done if the texture was created with the GPUTexture::kAutoMipmap flag set.
 *
 * @param face          Face to update, @see CubeFace.
 * @param data          New texture data.
 * @param updateMipmap  Whether to update mipmap images (defaults to true).
 */
void TextureCube::update(unsigned face, const void *data, bool updateMipmap) {
    check(face < CubeFace::kNumFaces);

    IntRect area(0, 0, size(), size());
    m_gpu->update(area, data, 0, face);

    /* Regenerate mipmaps if requested. */
    if (updateMipmap && m_gpu->flags() & GPUTexture::kAutoMipmap)
        m_gpu->generateMipmap();
}

/**
 * Update an area of the texture.
 *
 * Updates an area of the top mip level of the texture. The buffer must contain
 * pixel data in the same format as the texture, and it must equal the area
 * size specified.
 *
 * If updateMipmap is true (the default), the mipmap images of the texture
 * will be regenerated based on the updated image content. Note that this will
 * only actually be done if the texture was created with the
 * GPUTexture::kAutoMipmap flag set.
 *
 * @param face          Face to update, @see CubeFace.
 * @param area          Area to update.
 * @param data          New texture data.
 * @param updateMipmap  Whether to update mipmap images (defaults to true).
 */
void TextureCube::update(unsigned face, const IntRect &area, const void *data, bool updateMipmap) {
    check(face < CubeFace::kNumFaces);

    m_gpu->update(area, data, 0, face);

    /* Regenerate mipmaps if requested. */
    if (updateMipmap && m_gpu->flags() & GPUTexture::kAutoMipmap)
        m_gpu->generateMipmap();
}

/**
 * Update a specific mip level of the texture.
 *
 * Updates an area of a specific mip level of the texture. The buffer must
 * contain pixel data in the same format as the texture, and it must equal the
 * area size specified. No mipmap regeneration will be performed.
 *
 * @param face          Face to update, @see CubeFace.
 * @param mip           Mip level to update.
 * @param area          Area to update.
 * @param data          New texture data.
 */
void TextureCube::update(unsigned face, unsigned mip, const IntRect &area, const void *data) {
    check(face < CubeFace::kNumFaces);
    check(mip < mips());

    m_gpu->update(area, data, mip, face);
}

/**
 * Render texture implementation.
 */

/** Construct the render texture.
 * @param texture       Texture that this render texture belongs to.
 * @param layer         Layer that is being targeted. */
RenderTexture::RenderTexture(TextureBase *texture, unsigned layer) :
    RenderTarget (texture->gpu()->width(),
                  texture->gpu()->height(),
                  texture->format(),
                  kTextureMediumPriority),
    m_texture    (texture),
    m_layer      (layer)
{}

/** Get the target GPU render target descriptor.
 * @param ref           Image reference structure to fill in. */
void RenderTexture::getRenderTargetDesc(GPURenderTargetDesc &desc) const {
    desc = GPURenderTargetDesc(1);
    desc.colour[0].texture = m_texture->gpu();
    desc.colour[0].layer = m_layer;
}

/** Get the target GPU texture image reference.
 * @param ref           Image reference structure to fill in. */
void RenderTexture::getTextureImageRef(GPUTextureImageRef &ref) const {
    ref = GPUTextureImageRef(m_texture->gpu(), m_layer);
}
