/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               OpenGL state management.
 */

#pragma once

#include "gpu/state.h"

#include <GL/glew.h>

struct GLFeatures;

/**
 * OpenGL state cache.
 *
 * This class caches current OpenGL state, to avoid unnecessary API calls to
 * change state. GLGPUInterface holds an instance of it, GL state changes should
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
    int swapInterval;               /**< Current swap interval. */

    /** Render target/viewport state. */
    IntRect viewport;               /**< Current viewport. */
    glm::ivec2 currentRTSize;       /**< Current render target size. */

    /** Clear state. */
    glm::vec4 clearColour;
    float clearDepth;
    uint32_t clearStencil;

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

    void setSwapInterval(int interval);
    void setViewport(const IntRect &viewport);

    void setClearColour(const glm::vec4 &colour);
    void setClearDepth(float depth);
    void setClearStencil(uint32_t stencil);

    void enableBlend(bool enable);
    void setBlendEquation(GLenum equation);
    void setBlendFunc(GLenum sourceFactor, GLenum destFactor);

    void enableDepthTest(bool enable);
    void enableDepthWrite(bool enable);
    void setDepthFunc(GLenum func);

    void enableCullFace(bool enable);
    void setCullFace(GLenum mode);
    void enableDepthClamp(bool enable);

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
