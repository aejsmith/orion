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

/** Bind a pipeline for rendering.
 * @param pipeline      Pipeline to use. */
void GLGPUManager::bindPipeline(GPUPipeline *pipeline) {
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
    this->state.setViewport(viewport);
}

/** Set the scissor test parameters.
 * @param enable        Whether to enable scissor testing.
 * @param scissor       Scissor rectangle. */
void GLGPUManager::setScissor(bool enable, const IntRect &scissor) {
    this->state.enableScissorTest(enable);
    if (enable)
        this->state.setScissor(scissor);
}

/** End a frame and present it on screen. */
void GLGPUManager::endFrame() {
    /* On OS X, CGLFlushDrawable will swap whichever framebuffer is currently
     * active. So, to flush the main window, we must bind it here. */
    this->state.bindFramebuffer(GL_FRAMEBUFFER, 0);

    SDL_GL_SwapWindow(g_mainWindow->sdlWindow());
}

/** Clear rendering buffers.
 * @param buffers       Buffers to clear (bitmask of ClearBuffer values).
 * @param colour        Colour to clear to.
 * @param depth         Depth value to clear to.
 * @param stencil       Stencil value to clear to. */
void GLGPUManager::clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil) {
    GLbitfield mask = 0;

    if (buffers & ClearBuffer::kColourBuffer) {
        this->state.setClearColour(colour);
        mask |= GL_COLOR_BUFFER_BIT;
    }

    if (buffers & ClearBuffer::kDepthBuffer) {
        this->state.setClearDepth(depth);
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if (buffers & ClearBuffer::kStencilBuffer) {
        this->state.setClearStencil(stencil);
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    glClear(mask);
}

/** Draw primitives.
 * @param type          Primitive type to render.
 * @param _vertices     Vertex data to use.
 * @param indices       Index data to use (can be null). */
void GLGPUManager::draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) {
    GLVertexData *glVertices = static_cast<GLVertexData *>(vertices);

    /* Bind the VAO and the index buffer (if any). */
    glVertices->bind((indices) ? indices->buffer() : nullptr);

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
