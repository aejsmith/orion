/*
 * Copyright (C) 2015-2016 Alex Smith
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
 * @brief               OpenGL GPU interface implementation.
 */

#include "buffer.h"
#include "gl.h"
#include "pipeline.h"
#include "resource.h"
#include "texture.h"
#include "vertex_data.h"

#include "engine/engine.h"
#include "engine/window.h"

/**
 * Frame methods.
 */

/** End a frame and present it on screen. */
void GLGPUManager::endFrame() {
    /* On OS X, CGLFlushDrawable will swap whichever framebuffer is currently
     * active. So, to flush the main window, we must bind it here. */
    this->state.bindFramebuffer(GL_FRAMEBUFFER, 0);

    SDL_GL_SwapWindow(g_mainWindow->sdlWindow());
}

/**
 * Texture operations.
 */

/** Copy pixels from one texture to another.
 * @param source        Source texture image reference.
 * @param dest          Destination texture image reference.
 * @param sourcePos     Position in source texture to copy from.
 * @param destPos       Position in destination texture to copy to.
 * @param size          Size of area to copy. */
void GLGPUManager::blit(
    const GPUTextureImageRef &source,
    const GPUTextureImageRef &dest,
    glm::ivec2 sourcePos,
    glm::ivec2 destPos,
    glm::ivec2 size)
{
    check(!m_currentRenderPass);

    // TODO: use ARB_copy_image where supported.
    // TODO: validate dimensions? against correct mip level

    /* If copying a depth texture, both formats must match. */
    bool isDepth = source && PixelFormat::isDepth(source.texture->format());
    check(isDepth == (dest && PixelFormat::isDepth(dest.texture->format())));
    check(!isDepth || source.texture->format() == dest.texture->format());

    /* Preserve current framebuffer state. */
    GLuint prevDrawFBO = this->state.boundDrawFramebuffer;
    GLuint prevReadFBO = this->state.boundReadFramebuffer;

    /* Create a framebuffer for the source. */
    GLuint sourceFBO = 0;
    if (source) {
        GPURenderTargetDesc sourceTarget;
        if (isDepth) {
            sourceTarget.depthStencil = source;
        } else {
            sourceTarget.colour.resize(1);
            sourceTarget.colour[0] = source;
        }

        sourceFBO = createFBO(sourceTarget);
    }

    /* Bind the destination as the draw framebuffer. */
    GLuint destFBO = 0;
    if (dest) {
        GPURenderTargetDesc destTarget;
        if (isDepth) {
            destTarget.depthStencil = dest;
        } else {
            destTarget.colour.resize(1);
            destTarget.colour[0] = dest;
        }

        destFBO = createFBO(destTarget);
    }

    /* Bind the framebuffers. */
    this->state.bindFramebuffer(GL_DRAW_FRAMEBUFFER, destFBO);
    this->state.bindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);

    /* Blit the region. */
    glBlitFramebuffer(
        sourcePos.x, sourcePos.y,
        sourcePos.x + size.x, sourcePos.y + size.y,
        destPos.x, destPos.y,
        destPos.x + size.x, destPos.y + size.y,
        (isDepth) ? GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT,
        GL_NEAREST);

    /* Restore previous state. */
    this->state.bindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFBO);
    this->state.bindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFBO);
}

/**
 * Rendering methods.
 */

/** Begin a render pass.
 * @param desc          Descriptor for the render pass instance. */
void GLGPUManager::beginRenderPass(const GPURenderPassInstanceDesc &desc) {
    check(!m_currentRenderPass);

    /* Validate render pass state. */
    desc.pass->validateInstance(desc);

    m_currentRenderPass = desc.pass;
    m_currentRenderArea = desc.renderArea;

    /* Get an FBO for the render target and bind it. */
    GLuint fbo = (desc.targets.isMainWindow())
        ? 0
        : createFBO(desc.targets);
    this->state.bindFramebuffer(GL_FRAMEBUFFER, fbo);

    /* Set up default state for the pass. It's important to do this before
     * clearing as we need depth writes enabled for the clears below (the depth
     * mask affects glClearBuffer*). */
    setViewport(desc.renderArea);
    GPUManager::setBlendState<>();
    GPUManager::setDepthStencilState<>();
    GPUManager::setRasterizerState<>();

    const GPURenderPassDesc &passDesc = desc.pass->desc();

    /* We want to only clear the specified render area. Use scissor to do this. */
    auto configureScissor =
        [&] (const GPUTexture *texture) {
            bool needScissor =
                desc.renderArea.x != 0 ||
                desc.renderArea.y != 0 ||
                static_cast<uint32_t>(desc.renderArea.width) < texture->width() ||
                static_cast<uint32_t>(desc.renderArea.height) < texture->height();
            if (needScissor) {
                setScissor(true, desc.renderArea);
            } else {
                setScissor(false, IntRect());
            }
        };

    /* Clear the colour buffers which are specified to clear. */
    for (size_t i = 0; i < passDesc.colourAttachments.size(); i++) {
        const GPURenderAttachmentDesc &attachment = passDesc.colourAttachments[i];

        if (attachment.loadOp == GPURenderLoadOp::kClear) {
            configureScissor(desc.targets.colour[i].texture);
            glClearBufferfv(GL_COLOR, i, reinterpret_cast<const GLfloat *>(&desc.clearColours[i]));
        }
    }

    /* Clear depth/stencil buffers if required. */
    if (passDesc.depthStencilAttachment) {
        const GPURenderAttachmentDesc &attachment = passDesc.depthStencilAttachment;

        if (attachment.loadOp == GPURenderLoadOp::kClear || attachment.stencilLoadOp == GPURenderLoadOp::kClear)
            configureScissor(desc.targets.depthStencil.texture);

        if (attachment.loadOp == GPURenderLoadOp::kClear && attachment.stencilLoadOp == GPURenderLoadOp::kClear) {
            glClearBufferfi(GL_DEPTH_STENCIL, 0, desc.clearDepth, desc.clearStencil);
        } else if (attachment.loadOp == GPURenderLoadOp::kClear) {
            glClearBufferfv(GL_DEPTH, 0, &desc.clearDepth);
        } else if (attachment.stencilLoadOp == GPURenderLoadOp::kClear) {
            glClearBufferiv(GL_STENCIL, 0, reinterpret_cast<const GLint *>(&desc.clearStencil));
        }
    }

    /* Reset scissor state. */
    setScissor(false, IntRect());
}

/** End the current render pass. */
void GLGPUManager::endRenderPass() {
    check(m_currentRenderPass);
    m_currentRenderPass = nullptr;
}

/** Bind a pipeline for rendering.
 * @param pipeline      Pipeline to use. */
void GLGPUManager::bindPipeline(GPUPipeline *pipeline) {
    check(m_currentRenderPass);

    GLPipeline *glPipeline = static_cast<GLPipeline *>(pipeline);
    glPipeline->bind();
}

/** Bind a resource set.
 * @param index         Resource set index to bind to.
 * @param resources     Resource set to bind. */
void GLGPUManager::bindResourceSet(unsigned index, GPUResourceSet *resources) {
    for (unsigned slotIndex = 0; slotIndex < resources->slots().size(); slotIndex++) {
        const GPUResourceSet::Slot &slot = resources->slots()[slotIndex];

        if (!slot.object)
            continue;

        GLResourceSetLayout *layout = static_cast<GLResourceSetLayout *>(resources->layout());
        unsigned binding = layout->mapSlot(index, slotIndex);

        switch (slot.desc.type) {
            case GPUResourceType::kUniformBuffer:
            {
                GLBuffer *buffer = static_cast<GLBuffer *>(slot.object.get());
                buffer->bindIndexed(binding);
                break;
            }

            case GPUResourceType::kTexture:
            {
                GLTexture *texture = static_cast<GLTexture *>(slot.object.get());
                texture->bind(binding);
                GLSamplerState *sampler = static_cast<GLSamplerState *>(slot.sampler.get());
                sampler->bind(binding);
                break;
            }

            default:
                break;
        }
    }
}

/** Set the viewport.
 * @param viewport      Viewport rectangle in pixels. */
void GLGPUManager::setViewport(const IntRect &viewport) {
    check(m_currentRenderPass);
    check(m_currentRenderArea.contains(viewport));

    this->state.setViewport(viewport);
}

/** Set the scissor test parameters.
 * @param enable        Whether to enable scissor testing.
 * @param scissor       Scissor rectangle. */
void GLGPUManager::setScissor(bool enable, const IntRect &scissor) {
    check(m_currentRenderPass);

    this->state.enableScissorTest(enable);

    if (enable) {
        check(m_currentRenderArea.contains(scissor));
        this->state.setScissor(scissor);
    }
}

/** Draw primitives.
 * @param type          Primitive type to render.
 * @param _vertices     Vertex data to use.
 * @param indices       Index data to use (can be null). */
void GLGPUManager::draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) {
    check(m_currentRenderPass);

    GLVertexData *glVertices = static_cast<GLVertexData *>(vertices);

    /* Bind the VAO and the index buffer (if any). */
    glVertices->bind((indices) ? indices->buffer() : nullptr);

    check(this->state.boundPipeline);

    #if ORION_GL_VALIDATE_PROGRAMS
        GLint result = GL_FALSE;
        glValidateProgramPipeline(this->state.boundPipeline);
        glGetProgramPipelineiv(this->state.boundPipeline, GL_VALIDATE_STATUS, &result);
        if (result != GL_TRUE) {
            glGetProgramiv(this->state.boundPipeline, GL_INFO_LOG_LENGTH, &result);
            std::unique_ptr<char[]> log(new char[result]);
            glGetProgramPipelineInfoLog(this->state.boundPipeline, result, &result, log.get());

            logError("GL: Pipeline validation failed");
            logInfo("GL: Info log:\n%s", log.get());
            fatal("GL pipeline validation error (see log)");
        }
    #endif

    GLenum mode = GLUtil::convertPrimitiveType(type);
    if (indices) {
        /* FIXME: Check whether index type is supported (in generic code?) */
        glDrawElements(
            mode,
            indices->count(),
            GLUtil::convertIndexType(indices->type()),
            reinterpret_cast<void *>(indices->offset()));
    } else {
        glDrawArrays(mode, 0, vertices->count());
    }

    g_engine->stats().drawCalls++;
}

/**
 * Debug methods.
 */

#ifdef ORION_BUILD_DEBUG

/** Begin a debug group.
 * @param str           Group string. */
void GLGPUManager::beginDebugGroup(const std::string &str) {
    if (this->features[GLFeatures::kCapKHRDebug])
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, str.length(), str.c_str());
}

/** End the current debug group. */
void GLGPUManager::endDebugGroup() {
    if (this->features[GLFeatures::kCapKHRDebug])
        glPopDebugGroup();
}

#endif /* ORION_BUILD_DEBUG */
