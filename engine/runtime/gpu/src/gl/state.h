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
 * @brief               OpenGL state management.
 */

#pragma once

#include "gpu/state.h"

#ifdef ORION_PLATFORM_WIN32
    #define GLEW_STATIC
#endif

#include <GL/glew.h>

struct GLFeatures;

/**
 * OpenGL state cache.
 *
 * This class caches current OpenGL state, to avoid unnecessary API calls to
 * change state. GLGPUManager holds an instance of it, GL state changes should
 * be made by calling functions on that.
 *
 * When adding state to this structure, be sure to add default initializers to
 * the constructor.
 */
struct GLState {
    /** Class holding buffer bindings. */
    class BufferBindings {
    public:
        BufferBindings() :
            m_arrayBuffer(0),
            m_elementArrayBuffer(0),
            m_uniformBuffer(0)
        {}

        /** @return             Current binding for the specified target. */
        GLuint &operator [](GLenum target) {
            switch (target) {
                case GL_ARRAY_BUFFER:
                    return m_arrayBuffer;
                case GL_ELEMENT_ARRAY_BUFFER:
                    return m_elementArrayBuffer;
                case GL_UNIFORM_BUFFER:
                    return m_uniformBuffer;
                default:
                    unreachable();
            }
        }
    private:
        GLuint m_arrayBuffer;
        GLuint m_elementArrayBuffer;
        GLuint m_uniformBuffer;
    };

    /** Structure holding texture unit state. */
    struct TextureUnit {
        GLenum target;
        GLuint texture;
        GLuint sampler;
    public:
        TextureUnit() :
            target(GL_NONE),
            texture(0),
            sampler(0)
        {}
    };
public:
    /** Viewport state. */
    IntRect viewport;               /**< Current viewport. */

    /** Blending state. */
    bool blendEnabled;
    GLenum blendEquation;
    GLenum blendSourceFactor;
    GLenum blendDestFactor;

    /** Depth testing state. */
    bool depthTestEnabled;
    bool depthWriteEnabled;
    GLenum depthFunc;

    /** Rasterizer state. */
    bool cullFaceEnabled;
    GLenum cullFace;
    bool depthClampEnabled;

    /** Scissor test state. */
    bool scissorTestEnabled;
    IntRect scissor;

    /** Object bindings. */
    BufferBindings boundBuffers;
    GLuint boundDrawFramebuffer;
    GLuint boundReadFramebuffer;
    GLuint boundPipeline;
    unsigned activeTexture;
    TextureUnit *textureUnits;
    GLuint boundVertexArray;
public:
    GLState();
    ~GLState();

    void initResources(GLFeatures &features);

    void setViewport(const IntRect &viewport);

    void enableBlend(bool enable);
    void setBlendEquation(GLenum equation);
    void setBlendFunc(GLenum sourceFactor, GLenum destFactor);

    void enableDepthTest(bool enable);
    void enableDepthWrite(bool enable);
    void setDepthFunc(GLenum func);

    void enableCullFace(bool enable);
    void setCullFace(GLenum mode);
    void enableDepthClamp(bool enable);

    void enableScissorTest(bool enable);
    void setScissor(const IntRect &scissor);

    void bindBuffer(GLenum target, GLuint buffer);
    void bindBufferBase(GLenum target, GLuint index, GLuint buffer);
    void bindFramebuffer(GLenum target, GLuint framebuffer);
    void bindPipeline(GLuint pipeline);
    void bindSampler(unsigned unit, GLuint sampler);
    void bindTexture(unsigned unit, GLenum target, GLuint texture);
    void bindVertexArray(GLuint array);

    void invalidateBuffer(GLenum target, GLuint buffer);
    void invalidatePipeline(GLuint pipeline);
    void invalidateTexture(GLuint texture);
};

/**
 * GPU state objects.
 */

/** OpenGL blend state object implementation. */
struct GLBlendState : public GPUBlendState {
    bool enable;                    /**< Whether to enable blending. */
    GLenum blendEquation;           /**< GL blend equation. */
    GLenum sourceFactor;            /**< GL source factor. */
    GLenum destFactor;              /**< GL destination factor. */
public:
    GLBlendState(const GPUBlendStateDesc &desc) : GPUBlendState(desc) {}
};

/** OpenGL depth/stencil state object implementation. */
struct GLDepthStencilState : public GPUDepthStencilState {
    bool depthEnable;               /**< Whether to enable depth test. */
    GLenum depthFunc;               /**< Comparison function. */
public:
    GLDepthStencilState(const GPUDepthStencilStateDesc &desc) : GPUDepthStencilState(desc) {}
};

/** OpenGL rasterizer state object implementation. */
struct GLRasterizerState : public GPURasterizerState {
    GLenum cullMode;                /**< Face culling mode. */
public:
    GLRasterizerState(const GPURasterizerStateDesc &desc) : GPURasterizerState(desc) {}
};

/** OpenGL sampler state object implementation. */
class GLSamplerState : public GPUSamplerState {
public:
    GLSamplerState(const GPUSamplerStateDesc &desc);
    ~GLSamplerState();

    void bind(unsigned index);
private:
    GLuint m_sampler;               /**< Sampler object. */
};
