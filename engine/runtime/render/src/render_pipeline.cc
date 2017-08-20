/*
 * Copyright (C) 2017 Alex Smith
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
 * @brief               Rendering pipeline class.
 */

#include "engine/debug_manager.h"
#include "engine/serialiser.h"
#include "engine/window.h"

#include "render/post_effect.h"
#include "render/render_pipeline.h"
#include "render/render_view.h"

#include "render/post_effects/gamma_correction_effect.h"
#include "render/post_effects/tonemap_effect.h"

/** Global resources for all pipelines. */
static GlobalResource<RenderPipeline::BaseResources> g_renderPipelineResources;

/** Construct the pipeline. */
RenderPipeline::RenderPipeline() {
    /* Ensure global resources are initialised. */
    g_renderPipelineResources.init();

    /* Default to having a tonemapping and gamma correction pass. Really the
     * gamma correction should only be done if targetting the main window, for
     * off-screen rendering it would be better to just store the results in an
     * sRGB texture. */
    m_postEffects.emplace_back(new TonemapEffect);
    m_postEffects.emplace_back(new GammaCorrectionEffect);
}

/** Destroy the pipeline. */
RenderPipeline::~RenderPipeline() {}

/** Serialise the chain.
 * @param serialiser    Serialiser to write to. */
void RenderPipeline::serialise(Serialiser &serialiser) const {
    Object::serialise(serialiser);

    serialiser.beginArray("postEffects");

    for (PostEffect *effect : m_postEffects)
        serialiser.push(effect);

    serialiser.endArray();
}

/** Deserialise the chain.
 * @param serialiser    Serialiser to read from. */
void RenderPipeline::deserialise(Serialiser &serialiser) {
    Object::deserialise(serialiser);

    if (serialiser.beginArray("postEffects")) {
        m_postEffects.clear();

        ObjectPtr<PostEffect> effect;
        while (serialiser.pop(effect))
            m_postEffects.emplace_back(std::move(effect));

        serialiser.endArray();
    }
}

/** @return             Global resources for all pipelines. */
const RenderPipeline::BaseResources &RenderPipeline::resources() {
    return *g_renderPipelineResources;
}

/** Add a post-processing effect to the end of the list.
 * @param effect        Effect to add. */
void RenderPipeline::addPostEffect(ObjectPtr<PostEffect> effect) {
    m_postEffects.emplace_back(std::move(effect));
}

/**
 * Render all post-processing effects.
 *
 * Renders all post-processing effects (if any) and outputs the final image to
 * the render target.
 *
 * @param context       Rendering context.
 * @param input         Input texture.
 * @param imageType     Type of the input image.
 */
void RenderPipeline::renderPostEffects(RenderContext &context,
                                       const RenderTargetPool::Handle &input,
                                       ImageType imageType) const
{
    /* Helper to check that the render target format is suitable for the output
     * image type. */
    auto validateTargetImageType =
        [&] (const ImageType outputImageType) {
            #if ORION_BUILD_DEBUG
                GPURenderTargetDesc target;
                context.target().getRenderTargetDesc(target);
                const PixelFormat targetFormat = target.colour[0].texture->format();

                ImageType targetType;
                if (PixelFormat::isFloat(targetFormat)) {
                    targetType = ImageType::kHDR;
                } else if (PixelFormat::isSRGB(targetFormat)) {
                    targetType = ImageType::kLinearLDR;
                } else {
                    targetType = ImageType::kNonLinearLDR;
                }

                if (targetType != outputImageType) {
                    logWarning("Render target expects output image type %d but render pipeline output type is %d",
                               targetType,
                               outputImageType);
                }
            #endif
        };

    if (m_postEffects.empty()) {
        validateTargetImageType(imageType);

        /* Just blit to the output. */
        GPUTextureImageRef dest;
        context.target().getTextureImageRef(dest);
        g_gpuManager->blit(GPUTextureImageRef(input),
                           dest,
                           glm::ivec2(0, 0),
                           context.view().viewport().pos(),
                           context.view().viewport().size());
        return;
    }

    GPU_DEBUG_GROUP("Post Processing");

    /* We bounce between up to 2 temporary render targets. These are allocated
     * below when needed. TODO: Maybe we should have these allocated up front
     * when the render pipeline is created, based on requirements of the whole
     * effect chain. */
    RenderTargetPool::Handle dest;
    RenderTargetPool::Handle source = input;

    for (PostEffect *effect : m_postEffects) {
        GPU_DEBUG_GROUP("%s", effect->metaClass().name());

        const bool isLast = effect == m_postEffects.back();

        ImageType outputImageType = effect->outputImageType();
        if (outputImageType == ImageType::kDontCare)
            outputImageType = imageType;

        GPURenderTargetDesc targetDesc(1);
        IntRect targetArea;

        if (isLast) {
            validateTargetImageType(outputImageType);

            /* The final effect outputs onto the real render target. */
            context.target().getRenderTargetDesc(targetDesc);
            targetArea = context.view().viewport();
        } else {
            #if ORION_BUILD_DEBUG
                const ImageType expectedImageType = effect->inputImageType();
                if (expectedImageType != ImageType::kDontCare && expectedImageType != imageType) {
                    logWarning("Effect '%s' expects input image type %d but current type is %d",
                               effect->metaClass().name(),
                               expectedImageType,
                               imageType);
                }
            #endif

            if (!dest || imageType != outputImageType) {
                PixelFormat format;

                switch (outputImageType) {
                    case ImageType::kHDR:
                        format = kHDRColourBufferFormat;
                        break;

                    case ImageType::kLinearLDR:
                        format = kLinearLDRColourBufferFormat;
                        break;

                    case ImageType::kNonLinearLDR:
                        format = kNonLinearLDRColourBufferFormat;
                        break;

                    default:
                        unreachable();

                }

                auto textureDesc = GPUTextureDesc().
                    setType   (GPUTexture::kTexture2D).
                    setWidth  (input->width()).
                    setHeight (input->height()).
                    setMips   (1).
                    setFlags  (GPUTexture::kRenderTarget).
                    setFormat (format);

                dest = g_renderTargetPool->allocate(textureDesc);
            }

            targetDesc.colour[0].texture = dest;
            targetArea = IntRect(0, 0, input->width(), input->height());
        }

        effect->render(source, targetDesc, targetArea);

        /* Switch the output to be the source for the next effect. */
        std::swap(dest, source);

        /* Changing image type will invalidate the other target we have as well,
         * need to change the format. */
        if (outputImageType != imageType || dest == input)
            dest = nullptr;

        imageType = outputImageType;
    }
}

/** Render debug primitives.
 * @param context       Rendering context. */
void RenderPipeline::renderDebug(RenderContext &context) const {
    GPU_DEBUG_GROUP("Debug");

    GPURenderTargetDesc targetDesc;
    context.target().getRenderTargetDesc(targetDesc);

    GPUCommandList *cmdList = beginSimpleRenderPass(targetDesc,
                                                    context.view().viewport(),
                                                    GPURenderLoadOp::kLoad);

    /* Draw debug primitives onto the view. */
    g_debugManager->renderView(cmdList, context.view().getResources());

    g_gpuManager->submitRenderPass(cmdList);
}

/**
 * Begin a simple render pass.
 *
 * Begins a simple render pass with a single colour attachment. These render
 * passes are cached and will be reused.
 *
 * @param desc          Descriptor for the colour attachment.
 */
GPUCommandList* RenderPipeline::beginSimpleRenderPass(const GPURenderTargetDesc &target,
                                                      const IntRect &area,
                                                      GPURenderLoadOp loadOp)
{
    assert(target.colour.size() == 1);
    assert(!target.depthStencil);

    /* Get a render pass matching the target format. */
    GPURenderAttachmentDesc attachmentDesc;
    attachmentDesc.format = target.colour[0].texture->format();
    attachmentDesc.loadOp = loadOp;

    auto &cache = g_renderPipelineResources->renderPasses;

    auto it = cache.find(attachmentDesc);
    if (it == cache.end()) {
        GPURenderPassDesc passDesc(1);
        passDesc.colourAttachments[0] = attachmentDesc;

        GPURenderPassPtr pass = g_gpuManager->createRenderPass(std::move(passDesc));
        auto ret = cache.emplace(attachmentDesc, std::move(pass));
        check(ret.second);
        it = ret.first;
    }

    GPURenderPassInstanceDesc passDesc(it->second);
    passDesc.targets    = target;
    passDesc.renderArea = area;

    return g_gpuManager->beginRenderPass(passDesc);
}
