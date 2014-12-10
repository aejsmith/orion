/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               OpenGL GPU interface implementation.
 */

#include "buffer.h"
#include "gl.h"
#include "pipeline.h"
#include "texture.h"
#include "vertex_data.h"

#include "engine/window.h"

/** Bind a pipeline for rendering.
 * @param pipeline      Pipeline to use. */
void GLGPUInterface::bindPipeline(GPUPipeline *pipeline) {
    GLPipeline *glPipeline = static_cast<GLPipeline *>(pipeline);
    glPipeline->bind();
}

/** Bind a texture.
 * @param index         Texture unit index to bind to.
 * @param texture       Texture to bind.
 * @param sampler       Sampler state to bind with. */
void GLGPUInterface::bindTexture(unsigned index, GPUTexture *texture, GPUSamplerState *sampler) {
    GLTexture *glTexture = static_cast<GLTexture *>(texture);
    glTexture->bind(index);
    GLSamplerState *glSampler = static_cast<GLSamplerState *>(sampler);
    glSampler->bind(index);
}

/** Bind a uniform buffer.
 * @param index         Uniform block index to bind to.
 * @param buffer        Buffer to bind. */
void GLGPUInterface::bindUniformBuffer(unsigned index, GPUBuffer *buffer) {
    check(buffer->type() == GPUBuffer::kUniformBuffer);

    GLBuffer *glBuffer = static_cast<GLBuffer *>(buffer);
    glBuffer->bindIndexed(index);
}

/** Set the viewport.
 * @param viewport      Viewport rectangle in pixels. */
void GLGPUInterface::setViewport(const IntRect &viewport) {
    this->state.setViewport(viewport);
}

/** End a frame and present it on screen.
 * @param vsync         Whether to wait for vertical sync. */
void GLGPUInterface::endFrame(bool vsync) {
    /* On OS X, CGLFlushDrawable will swap whichever framebuffer is currently
     * active. So, to flush the main window, we must bind it here. */
    this->state.bindFramebuffer(GL_FRAMEBUFFER, 0);

    this->state.setSwapInterval(vsync);
    SDL_GL_SwapWindow(g_mainWindow->sdlWindow());
}

/** Clear rendering buffers.
 * @param buffers       Buffers to clear (bitmask of ClearBuffer values).
 * @param colour        Colour to clear to.
 * @param depth         Depth value to clear to.
 * @param stencil       Stencil value to clear to. */
void GLGPUInterface::clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil) {
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
void GLGPUInterface::draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) {
    GLVertexData *glVertices = static_cast<GLVertexData *>(vertices);

    /* Bind the VAO and the index buffer (if any). */
    glVertices->bind((indices) ? indices->buffer() : nullptr);

    GLenum mode = gl::convertPrimitiveType(type);
    if (indices) {
        /* FIXME: Check whether index type is supported (in generic code?) */
        glDrawElements(mode, indices->count(), gl::convertIndexType(indices->type()), nullptr);
    } else {
        glDrawArrays(mode, 0, vertices->count());
    }
}
