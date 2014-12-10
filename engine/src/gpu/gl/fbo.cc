/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               OpenGL FBO management.
 */

#include "gl.h"
#include "texture.h"

#include "engine/window.h"

/** Set an FBO attachment.
 * @param fbo           FBO to set in.
 * @param attachment    Attachment point to set.
 * @param texture       Texture reference. */
static void setAttachment(GLuint fbo, GLenum attachment, const GPUTextureImageRef &texture) {
    GLTexture *glTexture = static_cast<GLTexture *>(texture.texture);

    check(glTexture->flags() & GPUTexture::kRenderTarget);

    switch (glTexture->glTarget()) {
        case GL_TEXTURE_2D:
            glFramebufferTexture2D(
                GL_DRAW_FRAMEBUFFER,
                attachment,
                glTexture->glTarget(),
                glTexture->texture(),
                texture.mip);
            break;
        case GL_TEXTURE_CUBE_MAP:
            glFramebufferTexture2D(
                GL_DRAW_FRAMEBUFFER,
                attachment,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + texture.layer,
                glTexture->texture(),
                texture.mip);
            break;
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            glFramebufferTextureLayer(
                GL_DRAW_FRAMEBUFFER,
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
GLuint GLGPUInterface::createFBO(const GPURenderTargetDesc &desc) {
    /* See if we have a cached FBO available. */
    auto ret = m_fbos.find(desc);
    if (ret != m_fbos.end())
        return ret->second;

    /* We need to create a new one. */
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    this->state.bindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLenum buffers[kMaxColourRenderTargets];

    for (size_t i = 0; i < desc.numColours; i++) {
        setAttachment(fbo, GL_COLOR_ATTACHMENT0 + i, desc.colour[i]);
        buffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    glReadBuffer((desc.numColours > 0) ? buffers[0] : GL_NONE);
    glDrawBuffers(desc.numColours, buffers);

    if (desc.depthStencil.texture)
        setAttachment(fbo, GL_DEPTH_STENCIL_ATTACHMENT, desc.depthStencil);

    /* Check for error. */
    GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        fatal("GL framebuffer error 0x%x", status);

    /* Cache the new FBO. */
    m_fbos.insert(std::make_pair(desc, fbo));

    return fbo;
}

/** Set the render target.
 * @param desc          Render target descriptor.
 * @param viewport      Optional viewport rectangle. */
void GLGPUInterface::setRenderTarget(const GPURenderTargetDesc *desc, const IntRect *viewport) {
    if (!desc) {
        /* Main window. */
        this->state.bindFramebuffer(GL_FRAMEBUFFER, 0);
        this->state.currentRTSize.x = g_mainWindow->width();
        this->state.currentRTSize.y = g_mainWindow->height();

        if (viewport) {
            setViewport(*viewport);
        } else {
            setViewport(IntRect(0, 0, g_mainWindow->width(), g_mainWindow->height()));
        }

        return;
    }

    uint32_t width, height;

    /* Validate render target and determine the dimensions. */
    if (desc->numColours) {
        check(desc->numColours < kMaxColourRenderTargets);
        check(desc->colour[0].texture);

        width = desc->colour[0].texture->width();
        height = desc->colour[0].texture->height();

        for (size_t i = 1; i < desc->numColours; i++) {
            check(desc->colour[i].texture);
            check(desc->colour[i].texture->width() == width);
            check(desc->colour[i].texture->height() == height);
        }

        if (desc->depthStencil.texture) {
            check(desc->depthStencil.texture->width() == width);
            check(desc->depthStencil.texture->height() == height);
        }
    } else {
        check(desc->depthStencil.texture);

        width = desc->depthStencil.texture->width();
        height = desc->depthStencil.texture->height();
    }

    /* Create or get a matching FBO. */
    GLuint fbo = createFBO(*desc);

    /* Bind it. */
    this->state.bindFramebuffer(GL_FRAMEBUFFER, fbo);

    /* Set new viewport. */
    this->state.currentRTSize.x = width;
    this->state.currentRTSize.y = height;
    if (viewport) {
        setViewport(*viewport);
    } else {
        setViewport(IntRect(0, 0, width, height));
    }
}

/** Copy pixels from one texture to another.
 * @param source        Source texture image reference.
 * @param dest          Destination texture image reference.
 * @param sourcePos     Position in source texture to copy from.
 * @param destPos       Position in destination texture to copy to.
 * @param size          Size of area to copy. */
void GLGPUInterface::blit(
    const GPUTextureImageRef *source,
    const GPUTextureImageRef *dest,
    glm::ivec2 sourcePos,
    glm::ivec2 destPos,
    glm::ivec2 size)
{
    // TODO: use ARB_copy_image where supported.
    // TODO: validate dimensions? against correct mip level

    check(!source || source->texture);
    check(!dest || dest->texture);

    /* If copying a depth texture, both formats must match. */
    bool isDepth = source && PixelFormat::isDepth(source->texture->format());
    check(isDepth == (dest && PixelFormat::isDepth(dest->texture->format())));
    check(!isDepth || source->texture->format() == dest->texture->format());

    /* Preserve current framebuffer state. */
    GLuint prevDrawFBO = this->state.boundDrawFramebuffer;
    GLuint prevReadFBO = this->state.boundReadFramebuffer;

    /* Create a framebuffer for the source. */
    GLuint sourceFBO = 0;
    if (source) {
        GPURenderTargetDesc sourceTarget;
        if (isDepth) {
            sourceTarget.numColours = 0;
            sourceTarget.depthStencil = *source;
        } else {
            sourceTarget.numColours = 1;
            sourceTarget.colour[0] = *source;
        }

        sourceFBO = createFBO(sourceTarget);
    }

    /* Bind the destination as the draw framebuffer. */
    GLuint destFBO = 0;
    if (dest) {
        GPURenderTargetDesc destTarget;
        if (isDepth) {
            destTarget.numColours = 0;
            destTarget.depthStencil = *dest;
        } else {
            destTarget.numColours = 1;
            destTarget.colour[0] = *dest;
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

/** Invalidate FBOs referring to a texture.
 * @param texture       Texture being destroyed. */
void GLGPUInterface::invalidateFBOs(const GLTexture *texture) {
    for (auto it = m_fbos.begin(); it != m_fbos.end();) {
        const GPURenderTargetDesc &target = it->first;
        bool invalidate = false;

        if (target.depthStencil.texture == texture) {
            invalidate = true;
        } else {
            for (size_t i = 0; i < target.numColours; i++) {
                if (target.colour[i].texture == texture)
                    invalidate = true;
            }
        }

        if (invalidate) {
            glDeleteFramebuffers(1, &it->second);
            it = m_fbos.erase(it);
        } else {
            ++it;
        }
    }
}
