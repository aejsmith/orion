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

#pragma once

#include "state.h"

#include "core/hash_table.h"

#include "gpu/gpu_manager.h"

#include <SDL.h>

#include <array>
#include <set>

/** Define to 1 to enable ARB_debug_output. */
#define ORION_GL_DEBUG                  ORION_BUILD_DEBUG

/** Define to 1 to enable ARB_debug_output notification messages (excessive). */
#define ORION_GL_DEBUG_NOTIFICATIONS    0

/** Define to 1 to keep shader objects around, to allow examination in OpenGL Profiler. */
#define ORION_GL_KEEP_SHADER_OBJECTS    0

/** Define to 1 to validate programs on every draw. */
#define ORION_GL_VALIDATE_PROGRAMS      ORION_BUILD_DEBUG

#include "state.h"

class GLTexture;

/**
 * OpenGL feature information.
 *
 * TODO:
 *  - Add a bitmap of features that we often need to check for so we don't have
 *    to do a string comparison every time they are checked.
 */
struct GLFeatures {
    /** Capabilities of the GL implementation (e.g. frequently-checked extensions). */
    enum : uint32_t {
        /** GL_KHR_debug. */
        kCapKHRDebug = (1 << 0),
    };

    GLint versionMajor;                 /**< GL_MAJOR_VERSION. */
    GLint versionMinor;                 /**< GL_MINOR_VERSION. */

    /** List of extensions. */
    std::set<std::string> extensions;

    /** Capability flags. */
    uint32_t capabilities;

    /** Cached glGet* parameters. */
    GLfloat maxAnisotropy;              /**< GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT */
    GLint maxTextureUnits;              /**< GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS */

    /** Check whether an extension is supported.
     * @param extension     Extension to check for.
     * @return              Whether the extension is supported. */
    bool operator [](const std::string &extension) const {
        return this->extensions.find(extension) != this->extensions.end();
    }

    /** Check for a capability.
     * @param caps          Capability flags to check for.
     * @return              Whether the capability is supported. */
    bool operator [](uint32_t caps) const {
        return (this->capabilities & caps) == caps;
    }
};

/** Structure mapping PixelFormat to GL types. */
struct GLPixelFormat {
    GLenum internalFormat;              /**< Internal texture format. */
    GLenum format;                      /**< Pixel data format. */
    GLenum type;                        /**< Pixel data type. */
public:
    GLPixelFormat(GLenum i = GL_NONE, GLenum f = GL_NONE, GLenum t = GL_NONE) :
        internalFormat(i),
        format(f),
        type(t)
    {}
};

/** OpenGL GPU interface implementation. */
class GLGPUManager : public GPUManager, public GPUGenericCommandList::Context {
public:
    GLGPUManager(const EngineConfiguration &config, Window *&window);
    ~GLGPUManager();

    /**
     * GPU interface methods.
     */

    GPUBufferPtr createBuffer(const GPUBufferDesc &desc) override;
    GPUPipelinePtr createPipeline(GPUPipelineDesc &&desc) override;
    GPUTexturePtr createTexture(const GPUTextureDesc &desc) override;
    GPUTexturePtr createTextureView(const GPUTextureImageRef &image) override;
    GPUVertexDataPtr createVertexData(GPUVertexDataDesc &&desc) override;

    GPUBlendStatePtr createBlendState(const GPUBlendStateDesc &desc) override;
    GPUDepthStencilStatePtr createDepthStencilState(const GPUDepthStencilStateDesc &desc) override;
    GPURasterizerStatePtr createRasterizerState(const GPURasterizerStateDesc &desc) override;
    GPUSamplerStatePtr createSamplerState(const GPUSamplerStateDesc &desc) override;

    GPUResourceSetLayoutPtr createResourceSetLayout(GPUResourceSetLayoutDesc &&desc) override;
    GPUProgramPtr createProgram(GPUProgramDesc &&desc) override;

    void endFrame() override;

    void blit(const GPUTextureImageRef &source,
              const GPUTextureImageRef &dest,
              glm::ivec2 sourcePos,
              glm::ivec2 destPos,
              glm::ivec2 size) override;

    GPUCommandList *beginRenderPass(const GPURenderPassInstanceDesc &desc) override;
    void submitRenderPass(GPUCommandList *cmdList) override;

    /**
     * Command context interface.
     */

    void bindPipeline(GPUPipeline *pipeline) override;
    void bindResourceSet(unsigned index, GPUResourceSet *resources) override;
    void setBlendState(GPUBlendState *state) override;
    void setDepthStencilState(GPUDepthStencilState *state) override;
    void setRasterizerState(GPURasterizerState *state) override;
    void setViewport(const IntRect &viewport) override;
    void setScissor(bool enable, const IntRect &scissor) override;

    void draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) override;

    #ifdef ORION_BUILD_DEBUG
    void beginDebugGroup(const std::string &str) override;
    void endDebugGroup() override;
    #endif

    /**
     * Internal methods.
     */

    GLuint createFBO(const GPURenderTargetDesc &desc);
    void invalidateFBOs(const GLTexture *texture);

    /**
     * Public data.
     */

    GLFeatures features;                /**< GL feature information. */

    /** Mapping of engine pixel formats to GL types. */
    std::array<GLPixelFormat, PixelFormat::kNumFormats> pixelFormats;

    GLState state;                      /**< Cached GL state. */
    GLuint defaultVertexArray;          /**< Default VAO when no object-specific VAO is in use. */
private:
    void initFeatures();
    void initPixelFormats();

    static void GLEWAPIENTRY debugCallback(GLenum source,
                                           GLenum type,
                                           GLuint id,
                                           GLenum severity,
                                           GLsizei length,
                                           const GLchar *message,
                                           const GLvoid *param);

    SDL_GLContext m_sdlContext;         /**< SDL GL context. */

    /** Hash table of cached FBOs. */
    HashMap<GPURenderTargetDesc, GLuint> m_fbos;

    /**
     * Current render pass instance state.
     *
     * We don't need all of the render pass instance state so it would be
     * wasteful to store it all. Keep only the bits we need.
     */
    const GPURenderPass *m_currentRenderPass;
    glm::ivec2 m_currentRTSize;
    IntRect m_currentRenderArea;
};

extern GLGPUManager *g_opengl;

/**
 * Utility functions.
 */

namespace GLUtil {
    /** Convert a vertex attribute type to a GL type.
     * @param type          Attribute type.
     * @param gl            GL type.
     * @return              Whether type is supported. */
    static inline GLenum convertAttributeType(VertexAttribute::Type type) {
        switch (type) {
            case VertexAttribute::kByteType:
                return GL_BYTE;
            case VertexAttribute::kUnsignedByteType:
                return GL_UNSIGNED_BYTE;
            case VertexAttribute::kShortType:
                return GL_SHORT;
            case VertexAttribute::kUnsignedShortType:
                return GL_UNSIGNED_SHORT;
            case VertexAttribute::kIntType:
                return GL_INT;
            case VertexAttribute::kUnsignedIntType:
                return GL_UNSIGNED_INT;
            case VertexAttribute::kFloatType:
                return GL_FLOAT;
            case VertexAttribute::kDoubleType:
                return GL_DOUBLE;
            default:
                return 0;
        }
    }

    /** Convert a blend function to a GL blend equation.
     * @param func          Function to convert.
     * @return              OpenGL blend equation. */
    static inline GLenum convertBlendFunc(BlendFunc func) {
        switch (func) {
            case BlendFunc::kAdd:
                return GL_FUNC_ADD;
            case BlendFunc::kSubtract:
                return GL_FUNC_SUBTRACT;
            case BlendFunc::kReverseSubtract:
                return GL_FUNC_REVERSE_SUBTRACT;
            case BlendFunc::kMin:
                return GL_MIN;
            case BlendFunc::kMax:
                return GL_MAX;
            default:
                return 0;
        }
    }

    /** Convert a blend factor to a GL blend factor.
     * @param func          Function to convert.
     * @return              OpenGL blend equation. */
    static inline GLenum convertBlendFactor(BlendFactor factor) {
        switch (factor) {
            case BlendFactor::kZero:
                return GL_ZERO;
            case BlendFactor::kOne:
                return GL_ONE;
            case BlendFactor::kSourceColour:
                return GL_SRC_COLOR;
            case BlendFactor::kDestColour:
                return GL_DST_COLOR;
            case BlendFactor::kOneMinusSourceColour:
                return GL_ONE_MINUS_SRC_COLOR;
            case BlendFactor::kOneMinusDestColour:
                return GL_ONE_MINUS_DST_COLOR;
            case BlendFactor::kSourceAlpha:
                return GL_SRC_ALPHA;
            case BlendFactor::kDestAlpha:
                return GL_DST_ALPHA;
            case BlendFactor::kOneMinusSourceAlpha:
                return GL_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::kOneMinusDestAlpha:
                return GL_ONE_MINUS_DST_ALPHA;
            default:
                return 0;
        }
    }

    /** Convert a buffer type to a GL buffer target.
     * @param type          Buffer type.
     * @return              GL buffer target. */
    static inline GLenum convertBufferType(GPUBuffer::Type type) {
        switch (type) {
            case GPUBuffer::kVertexBuffer:
                return GL_ARRAY_BUFFER;
            case GPUBuffer::kIndexBuffer:
                return GL_ELEMENT_ARRAY_BUFFER;
            case GPUBuffer::kUniformBuffer:
                return GL_UNIFORM_BUFFER;
            default:
                return 0;
        }
    }

    /** Convert a buffer usage hint to a GL usage hint.
     * @param usage         Usage hint.
     * @return              OpenGL usage hint value. */
    static inline GLenum convertBufferUsage(GPUBuffer::Usage usage) {
        switch (usage) {
            case GPUBuffer::kStaticUsage:
                return GL_STATIC_DRAW;
            case GPUBuffer::kDynamicUsage:
                return GL_DYNAMIC_DRAW;
            case GPUBuffer::kTransientUsage:
                return GL_STREAM_DRAW;
            default:
                return 0;
        }
    }

    /** Convert a comparison function to a GL comparison function.
     * @param func          Function to convert.
     * @return              GL comparison function. */
    static inline GLenum convertComparisonFunc(ComparisonFunc func) {
        switch (func) {
            case ComparisonFunc::kAlways:
                return GL_ALWAYS;
            case ComparisonFunc::kNever:
                return GL_NEVER;
            case ComparisonFunc::kEqual:
                return GL_EQUAL;
            case ComparisonFunc::kNotEqual:
                return GL_NOTEQUAL;
            case ComparisonFunc::kLess:
                return GL_LESS;
            case ComparisonFunc::kLessOrEqual:
                return GL_LEQUAL;
            case ComparisonFunc::kGreater:
                return GL_GREATER;
            case ComparisonFunc::kGreaterOrEqual:
                return GL_GEQUAL;
            default:
                return 0;
        }
    }

    /** Convert a cull mode to a GL culling mode.
     * @param mode          Mode to convert.
     * @return              GL culling mode. */
    static inline GLenum convertCullMode(CullMode mode) {
        switch (mode) {
            case CullMode::kFront:
                return GL_FRONT;
            case CullMode::kBack:
                return GL_BACK;
            default:
                return 0;
        }
    }

    /** Convert an index data type to a GL data type.
     * @param type          Type to convert.
     * @return              GL data type. */
    static inline GLenum convertIndexType(GPUIndexData::Type type) {
        switch (type) {
            case GPUIndexData::kUnsignedShortType:
                return GL_UNSIGNED_SHORT;
            case GPUIndexData::kUnsignedIntType:
                return GL_UNSIGNED_INT;
            default:
                return 0;
        }
    }

    /** Convert a primitive type to a GL primitive type.
     * @param type          Type to convert.
     * @return              GL primitive type. */
    static inline GLenum convertPrimitiveType(PrimitiveType type) {
        switch (type) {
            case PrimitiveType::kTriangleList:
                return GL_TRIANGLES;
            case PrimitiveType::kTriangleStrip:
                return GL_TRIANGLE_STRIP;
            case PrimitiveType::kTriangleFan:
                return GL_TRIANGLE_FAN;
            case PrimitiveType::kPointList:
                return GL_POINTS;
            case PrimitiveType::kLineList:
                return GL_LINES;
            default:
                return 0;
        }
    }

    /** Convert a sampler address mode to a GL wrap mode.
     * @param mode          Mode to convert.
     * @return              Converted GL mode. */
    static inline GLint convertSamplerAddressMode(SamplerAddressMode mode) {
        switch (mode) {
            case SamplerAddressMode::kWrap:
                return GL_REPEAT;
            default:
                return GL_CLAMP_TO_EDGE;
        }
    }

    /** Convert a shader stage type to a GL shader type.
     * @param stage         Stage type to convert.
     * @return              Converted GL type. */
    static inline GLenum convertShaderStage(unsigned stage) {
        switch (stage) {
            case ShaderStage::kVertex:
                return GL_VERTEX_SHADER;
            case ShaderStage::kFragment:
                return GL_FRAGMENT_SHADER;
            default:
                return 0;
        }
    }

    /** Convert a shader stage type to a GL bitfield type.
     * @param stage         Stage type to convert.
     * @return              Converted GL bitfield type. */
    static inline GLbitfield convertShaderStageBitfield(unsigned stage) {
        switch (stage) {
            case ShaderStage::kVertex:
                return GL_VERTEX_SHADER_BIT;
            case ShaderStage::kFragment:
                return GL_FRAGMENT_SHADER_BIT;
            default:
                return 0;
        }
    }

    /** Convert a texture type to a GL texture target.
     * @param type          Texture type.
     * @return              GL texture target. */
    static inline GLenum convertTextureType(GPUTexture::Type type) {
        switch (type) {
            case GPUTexture::kTexture2D:
                return GL_TEXTURE_2D;
            case GPUTexture::kTexture2DArray:
                return GL_TEXTURE_2D_ARRAY;
            case GPUTexture::kTextureCube:
                return GL_TEXTURE_CUBE_MAP;
            case GPUTexture::kTexture3D:
                return GL_TEXTURE_3D;
            default:
                return 0;
        }
    }
}
