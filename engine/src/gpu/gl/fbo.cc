/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL FBO management.
 */

#include "gl.h"
#include "texture.h"

#include "engine/window.h"

/** Set an FBO attachment.
 * @param fbo		FBO to set in.
 * @param attachment	Attachment point to set.
 * @param texture	Texture reference. */
static void setAttachment(GLuint fbo, GLenum attachment, const GPUTextureImageRef &texture) {
	GLTexture *glTexture = static_cast<GLTexture *>(texture.texture);

	switch(glTexture->glTarget()) {
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

/** Set the render target.
 * @param target	Render target descriptor. */
void GLGPUInterface::setRenderTarget(const GPURenderTargetDesc *target) {
	if(!target) {
		/* Main window. */
		this->state.bindFramebuffer(GL_FRAMEBUFFER, 0);
		this->state.currentRTSize.x = g_mainWindow->width();
		this->state.currentRTSize.y = g_mainWindow->height();
		this->state.setViewport(IntRect(0, 0, g_mainWindow->width(), g_mainWindow->height()));
		return;
	}

	uint32_t width, height;

	/* Validate render target and determine the dimensions. */
	if(target->numColours) {
		check(target->numColours < kMaxColourRenderTargets);
		check(target->colour[0].texture);

		width = target->colour[0].texture->width();
		height = target->colour[0].texture->height();

		for(size_t i = 1; i < target->numColours; i++) {
			check(target->colour[i].texture);
			check(target->colour[i].texture->width() == width);
			check(target->colour[i].texture->height() == height);
		}

		if(target->depthStencil.texture) {
			check(target->depthStencil.texture->width() == width);
			check(target->depthStencil.texture->height() == height);
		}
	} else {
		check(target->depthStencil.texture);

		width = target->depthStencil.texture->width();
		height = target->depthStencil.texture->height();
	}

	/* See if we have a cached FBO available. */
	auto ret = m_fbos.find(*target);
	if(ret != m_fbos.end()) {
		this->state.bindFramebuffer(GL_FRAMEBUFFER, ret->second);
	} else {
		/* We need to create a new one. */
		GLuint fbo;
		glGenFramebuffers(1, &fbo);
		this->state.bindFramebuffer(GL_FRAMEBUFFER, fbo);

		GLenum buffers[kMaxColourRenderTargets];

		for(size_t i = 0; i < target->numColours; i++) {
			setAttachment(fbo, GL_COLOR_ATTACHMENT0 + i, target->colour[i]);
			buffers[i] = GL_COLOR_ATTACHMENT0 + i;
		}

		glReadBuffer(buffers[0]);
		glDrawBuffers(target->numColours, buffers);

		if(target->depthStencil.texture)
			setAttachment(fbo, GL_DEPTH_STENCIL_ATTACHMENT, target->depthStencil);

		/* Check for error. */
	        GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        	if(status != GL_FRAMEBUFFER_COMPLETE)
                	fatal("GL framebuffer error 0x%x", status);

		m_fbos.insert(std::make_pair(*target, fbo));
	}

	/* Set new viewport. */
	this->state.currentRTSize.x = width;
	this->state.currentRTSize.y = height;
	this->state.setViewport(IntRect(0, 0, width, height));
}

/** Invalidate FBOs referring to a texture.
 * @param texture	Texture being destroyed. */
void GLGPUInterface::invalidateFBOs(const GLTexture *texture) {
	for(auto it = m_fbos.begin(); it != m_fbos.end();) {
		const GPURenderTargetDesc &target = it->first;
		bool invalidate = false;

		if(target.depthStencil.texture == texture) {
			invalidate = true;
		} else {
			for(size_t i = 0; i < target.numColours; i++) {
				if(target.colour[i].texture == texture)
					invalidate = true;
			}
		}

		if(invalidate) {
			glDeleteFramebuffers(1, &it->second);
			it = m_fbos.erase(it);
		} else {
			++it;
		}
	}
}
