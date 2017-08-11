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
 * @brief               OpenGL FBO management.
 */

#include "gl.h"
#include "texture.h"

#include "engine/window.h"

/** Set an FBO attachment.
 * @param attachment    Attachment point to set.
 * @param texture       Texture reference. */
static void setAttachment(GLenum attachment, const GPUTextureImageRef &texture) {
    GLTexture *glTexture = static_cast<GLTexture *>(texture.texture);

    switch (glTexture->glTarget()) {
        case GL_TEXTURE_2D:
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                                   attachment,
                                   glTexture->glTarget(),
                                   glTexture->texture(),
                                   texture.mip);
            break;
        case GL_TEXTURE_CUBE_MAP:
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                                   attachment,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + texture.layer,
                                   glTexture->texture(),
                                   texture.mip);
            break;
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER,
                                      attachment,
                                      glTexture->texture(),
                                      texture.mip,
                                      texture.layer);
            break;
        default:
            fatal("Unhandled texture render target type");
    }
}

/** Create a framebuffer object for a render target descriptor.
 * @note                May trash current FBO binding state.
 * @param desc          Render target descriptor. Should be validated.
 * @return              Created framebuffer object. */
GLuint GLGPUManager::createFBO(const GPURenderTargetDesc &desc) {
    /* See if we have a cached FBO available. */
    auto ret = m_fbos.find(desc);
    if (ret != m_fbos.end())
        return ret->second;

    /* We need to create a new one. */
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    this->state.bindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLenum buffers[kMaxColourRenderTargets];

    for (size_t i = 0; i < desc.colour.size(); i++) {
        setAttachment(GL_COLOR_ATTACHMENT0 + i, desc.colour[i]);
        buffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    glReadBuffer((desc.colour.size() > 0) ? buffers[0] : GL_NONE);
    glDrawBuffers(desc.colour.size(), buffers);

    if (desc.depthStencil.texture) {
        if (PixelFormat::isDepthStencil(desc.depthStencil.texture->format())) {
            setAttachment(GL_DEPTH_STENCIL_ATTACHMENT, desc.depthStencil);
        } else {
            setAttachment(GL_DEPTH_ATTACHMENT, desc.depthStencil);
        }
    }

    /* Check for error. */
    GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        fatal("GL framebuffer error 0x%x", status);

    /* Cache the new FBO. */
    m_fbos.insert(std::make_pair(desc, fbo));

    return fbo;
}

/** Invalidate FBOs referring to a texture.
 * @param texture       Texture being destroyed. */
void GLGPUManager::invalidateFBOs(const GLTexture *texture) {
    for (auto it = m_fbos.begin(); it != m_fbos.end();) {
        const GPURenderTargetDesc &target = it->first;
        bool invalidate = false;

        if (target.depthStencil.texture == texture) {
            invalidate = true;
        } else {
            for (size_t i = 0; i < target.colour.size(); i++) {
                if (target.colour[i].texture == texture)
                    invalidate = true;
            }
        }

        if (invalidate) {
            if (this->state.boundDrawFramebuffer == it->second || this->state.boundReadFramebuffer == it->second)
                this->state.bindFramebuffer(GL_FRAMEBUFFER, 0);

            glDeleteFramebuffers(1, &it->second);
            it = m_fbos.erase(it);
        } else {
            ++it;
        }
    }
}
