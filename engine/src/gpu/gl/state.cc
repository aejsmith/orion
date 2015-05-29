/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               OpenGL state management.
 */

#include "gl.h"

/**
 * GL state caching.
 */

/**
 * Initalize the GL state.
 *
 * This initializes the state object with default OpenGL state. Check the OpenGL
 * specifications to determine the correct default values when adding new
 * entries here.
 */
GLState::GLState() :
    swapInterval(0),
    clearColour(0.0f, 0.0f, 0.0f, 0.0f),
    clearDepth(1.0f),
    clearStencil(0.0f),
    blendEnabled(false),
    blendEquation(GL_FUNC_ADD),
    blendSourceFactor(GL_ONE),
    blendDestFactor(GL_ZERO),
    depthTestEnabled(false),
    depthWriteEnabled(true),
    depthFunc(GL_LESS),
    cullFaceEnabled(false),
    cullFace(GL_BACK),
    depthClampEnabled(false),
    boundDrawFramebuffer(0),
    boundReadFramebuffer(0),
    boundPipeline(0),
    activeTexture(0),
    textureUnits(nullptr),
    boundVertexArray(0)
{}

/** Destroy the GL state. */
GLState::~GLState() {
    if (this->textureUnits)
        delete[] this->textureUnits;
}

/** Allocate arrays dependent on GL implementation capabilities.
 * @param features      GL features description. */
void GLState::initResources(GLFeatures &features) {
    this->textureUnits = new TextureUnit[features.maxTextureUnits];
}

/** Set the current swap interval.
 * @param interval      Interval to set (passed to SDL_GL_SetSwapInterval). */
void GLState::setSwapInterval(int interval) {
    if (interval != this->swapInterval) {
        SDL_GL_SetSwapInterval(interval);
        this->swapInterval = interval;
    }
}

/** Set the viewport.
 * @param viewport      Viewport to set. */
void GLState::setViewport(const IntRect &viewport) {
    if (viewport != this->viewport) {
        glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
        this->viewport = viewport;
    }
}

/** Set the colour clear value.
 * @param colour        Colour clear value. */
void GLState::setClearColour(const glm::vec4 &colour) {
    if (colour != this->clearColour) {
        glClearColor(colour.r, colour.g, colour.b, colour.a);
        this->clearColour = colour;
    }
}

/** Set the depth clear value.
 * @param depth         Depth clear value. */
void GLState::setClearDepth(float depth) {
    if (depth != this->clearDepth) {
        glClearDepth(depth);
        this->clearDepth = depth;
    }
}

/** Set the stencil clear value.
 * @param stencil       Stencil clear value. */
void GLState::setClearStencil(uint32_t stencil) {
    if (stencil != this->clearStencil) {
        glClearStencil(stencil);
        this->clearStencil = stencil;
    }
}

/** Set whether blending is enabled.
 * @param enable        Whether to enable blending. */
void GLState::enableBlend(bool enable) {
    if (enable != this->blendEnabled) {
        if (enable) {
            glEnable(GL_BLEND);
        } else {
            glDisable(GL_BLEND);
        }

        this->blendEnabled = enable;
    }
}

/** Set the blend equation.
 * @param equation      Blending equation. */
void GLState::setBlendEquation(GLenum equation) {
    if (equation != this->blendEquation) {
        glBlendEquation(equation);
        this->blendEquation = equation;
    }
}

/** Set the blending factors.
 * @param sourceFactor  Source factor.
 * @param destFactor    Destination factor. */
void GLState::setBlendFunc(GLenum sourceFactor, GLenum destFactor) {
    if (sourceFactor != this->blendSourceFactor || destFactor != this->blendDestFactor) {
        glBlendFunc(sourceFactor, destFactor);
        this->blendSourceFactor = sourceFactor;
        this->blendDestFactor = destFactor;
    }
}

/** Set whether the depth test is enabled.
 * @param enable        Whether to enable the depth test. */
void GLState::enableDepthTest(bool enable) {
    if (enable != this->depthTestEnabled) {
        if (enable) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }

        this->depthTestEnabled = enable;
    }
}

/** Set whether depth buffer writes are enabled.
 * @param enable        Whether to enable depth buffer writes. */
void GLState::enableDepthWrite(bool enable) {
    if (enable != this->depthWriteEnabled) {
        glDepthMask(enable);
        this->depthWriteEnabled = enable;
    }
}

/** Set the depth comparison function.
 * @param func          Depth comparison function. */
void GLState::setDepthFunc(GLenum func) {
    if (func != this->depthFunc) {
        glDepthFunc(func);
        this->depthFunc = func;
    }
}

/** Set whether face culling is enabled.
 * @param enable        Whether to enable face culling. */
void GLState::enableCullFace(bool enable) {
    if (enable != this->cullFaceEnabled) {
        if (enable) {
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }

        this->cullFaceEnabled = enable;
    }
}

/** Set the face culling mode.
 * @param mode          Face culling mode. */
void GLState::setCullFace(GLenum mode) {
    if (mode != this->cullFace) {
        glCullFace(mode);
        this->cullFace = mode;
    }
}

/** Set whether depth clamping is enabled.
 * @param enable        Whether to enable depth clamping. */
void GLState::enableDepthClamp(bool enable) {
    if (enable != this->depthClampEnabled) {
        if (enable) {
            glEnable(GL_DEPTH_CLAMP);
        } else {
            glDisable(GL_DEPTH_CLAMP);
        }

        this->depthClampEnabled = enable;
    }
}

/** Bind a buffer.
 * @param target        Target to bind buffer to.
 * @param buffer        Buffer to bind. */
void GLState::bindBuffer(GLenum target, GLuint buffer) {
    if (target == GL_ELEMENT_ARRAY_BUFFER) {
        /* Since the element array buffer binding is part of VAO state, make
         * sure we are on the default VAO. All element array buffer bindings
         * done outside of GLVertexData::bind() should be done on the default
         * VAO so that we don't affect the per-object VAOs and so that we can
         * keep track of the currently bound buffer more easily. */
        bindVertexArray(g_opengl->defaultVertexArray);
    }

    if (this->boundBuffers[target] != buffer) {
        glBindBuffer(target, buffer);
        this->boundBuffers[target] = buffer;
    }
}

/** Bind a buffer to an indexed buffer target.
 * @param target        Target to bind buffer to.
 * @param index         Binding point index.
 * @param buffer        Buffer to bind. */
void GLState::bindBufferBase(GLenum target, GLuint index, GLuint buffer) {
    /* Brain damaged API design alert! glBindBufferBase also binds to the
     * generic buffer binding point. */
    // TODO: Cache these
    glBindBufferBase(target, index, buffer);
    this->boundBuffers[target] = buffer;
}

/** Bind a framebuffer.
 * @param target        Target(s) to bind to.
 * @param framebuffer   Framebuffer to bind. */
void GLState::bindFramebuffer(GLenum target, GLuint framebuffer) {
    switch (target) {
        case GL_FRAMEBUFFER:
            if (this->boundDrawFramebuffer != framebuffer || this->boundReadFramebuffer != framebuffer) {
                glBindFramebuffer(target, framebuffer);
                this->boundDrawFramebuffer = framebuffer;
                this->boundReadFramebuffer = framebuffer;
            }

            break;
        case GL_DRAW_FRAMEBUFFER:
            if (this->boundDrawFramebuffer != framebuffer) {
                glBindFramebuffer(target, framebuffer);
                this->boundDrawFramebuffer = framebuffer;
            }

            break;
        case GL_READ_FRAMEBUFFER:
            if (this->boundReadFramebuffer != framebuffer) {
                glBindFramebuffer(target, framebuffer);
                this->boundReadFramebuffer = framebuffer;
            }

            break;
    }
}

/** Bind a program pipeline.
 * @param pipeline      Pipeline object to bind. */
void GLState::bindPipeline(GLuint pipeline) {
    if (this->boundPipeline != pipeline) {
        glBindProgramPipeline(pipeline);
        this->boundPipeline = pipeline;
    }
}

/** Bind a sampler to a texture unit.
 * @param unit          Texture unit to bind to. Note this is specified as the
 *                      unit index, not as a GL_TEXTUREn constant.
 * @param sampler       Texture sampler to bind. */
void GLState::bindSampler(unsigned unit, GLuint sampler) {
    TextureUnit &unitState = this->textureUnits[unit];
    if (unitState.sampler != sampler) {
        glBindSampler(unit, sampler);
        unitState.sampler = sampler;
    }
}

/**
 * Bind a texture to a texture unit.
 *
 * Makes the specified texture unit active and binds the given texture to it.
 * Although technically you can bind multiple textures with different targets
 * to the same texture unit, bad things are likely to happen if this is done,
 * so we don't allow it - we only bind one texture at a time to a unit.
 *
 * @param unit          Texture unit to bind to. Note this is specified as the
 *                      unit index, not as a GL_TEXTUREn constant.
 * @param target        Texture target to bind.
 * @param texture       Texture object to bind.
 */
void GLState::bindTexture(unsigned unit, GLenum target, GLuint texture) {
    if (this->activeTexture != unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        this->activeTexture = unit;
    }

    TextureUnit &unitState = this->textureUnits[unit];
    if (unitState.target != target || unitState.texture != texture) {
        if (unitState.target != GL_NONE && unitState.target != target) {
            /* Unbind the texture currently bound so that we don't have multiple
             * textures bound to different targets. */
            glBindTexture(unitState.target, 0);
        }

        glBindTexture(target, texture);
        unitState.target = target;
        unitState.texture = texture;
    }
}

/** Bind a VAO.
 * @param array         VAO to bind. */
void GLState::bindVertexArray(GLuint array) {
    if (array != this->boundVertexArray) {
        glBindVertexArray(array);
        this->boundVertexArray = array;
    }
}

/** Remove any cached bindings for a buffer being deleted.
 * @param target        Target type for the buffer.
 * @param buffer        Buffer being deleted. */
void GLState::invalidateBuffer(GLenum target, GLuint buffer) {
    if (this->boundBuffers[target] == buffer)
        this->boundBuffers[target] = 0;
}

/** Remove any cached bindings for a pipeline being deleted.
 * @param pipeline      Pipeline being deleted. */
void GLState::invalidatePipeline(GLuint pipeline) {
    if (this->boundPipeline == pipeline)
        this->boundPipeline = 0;
}

/** Remove any cached bindings for a texture being deleted.
 * @param texture       Texture being deleted. */
void GLState::invalidateTexture(GLuint texture) {
    for (GLint i = 0; i < g_opengl->features.maxTextureUnits; i++) {
        if (this->textureUnits[i].texture == texture)
            this->textureUnits[i].texture = 0;
    }
}

/**
 * State object management.
 */

/** Create a blend state object.
 * @param desc          Descriptor for blend state.
 * @return              Created blend state object. */
GPUBlendStatePtr GLGPUInterface::createBlendState(const GPUBlendStateDesc &desc) {
    auto exist = m_blendStates.find(desc);
    if (exist != m_blendStates.end())
        return exist->second;

    GLBlendState *state = new GLBlendState(desc);

    state->enable =
        desc.func != BlendFunc::kAdd ||
        desc.sourceFactor != BlendFactor::kOne ||
        desc.destFactor != BlendFactor::kZero;
    state->blendEquation = GLUtil::convertBlendFunc(desc.func);
    state->sourceFactor = GLUtil::convertBlendFactor(desc.sourceFactor);
    state->destFactor = GLUtil::convertBlendFactor(desc.destFactor);

    auto ret = m_blendStates.insert(std::make_pair(desc, GPUBlendStatePtr(state)));
    return ret.first->second;
}

/** Set the blend state.
 * @param state         Blend state to set. */
void GLGPUInterface::setBlendState(GPUBlendState *state) {
    GLBlendState *glState = static_cast<GLBlendState *>(state);

    this->state.enableBlend(glState->enable);
    this->state.setBlendEquation(glState->blendEquation);
    this->state.setBlendFunc(glState->sourceFactor, glState->destFactor);
}

/** Create a depth/stencil state object.
 * @param desc          Descriptor for depth/stencil state.
 * @return              Created depth/stencil state object. */
GPUDepthStencilStatePtr GLGPUInterface::createDepthStencilState(const GPUDepthStencilStateDesc &desc) {
    auto exist = m_depthStencilStates.find(desc);
    if (exist != m_depthStencilStates.end())
        return exist->second;

    GLDepthStencilState *state = new GLDepthStencilState(desc);

    /* Documentation for glDepthFunc: "Even if the depth buffer exists and the
     * depth mask is non-zero, the depth buffer is not updated if the depth test
     * is disabled". */
    state->depthEnable = desc.depthFunc != ComparisonFunc::kAlways || desc.depthWrite;
    state->depthFunc = GLUtil::convertComparisonFunc(desc.depthFunc);

    auto ret = m_depthStencilStates.insert(std::make_pair(desc, GPUDepthStencilStatePtr(state)));
    return ret.first->second;
}

/** Set the depth/stencil state.
 * @param state         State to set. */
void GLGPUInterface::setDepthStencilState(GPUDepthStencilState *state) {
    GLDepthStencilState *glState = static_cast<GLDepthStencilState *>(state);
    const GPUDepthStencilStateDesc &desc = state->desc();

    this->state.enableDepthTest(glState->depthEnable);
    this->state.enableDepthWrite(desc.depthWrite);
    this->state.setDepthFunc(glState->depthFunc);
}

/** Create a rasterizer state object.
 * @param desc          Descriptor for rasterizer state.
 * @return              Created rasterizer state object. */
GPURasterizerStatePtr GLGPUInterface::createRasterizerState(const GPURasterizerStateDesc &desc) {
    auto exist = m_rasterizerStates.find(desc);
    if (exist != m_rasterizerStates.end())
        return exist->second;

    GLRasterizerState *state = new GLRasterizerState(desc);

    state->cullMode = GLUtil::convertCullMode(desc.cullMode);

    auto ret = m_rasterizerStates.insert(std::make_pair(desc, GPURasterizerStatePtr(state)));
    return ret.first->second;
}

/** Set the rasterizer state.
 * @param state         State to set. */
void GLGPUInterface::setRasterizerState(GPURasterizerState *state) {
    GLRasterizerState *glState = static_cast<GLRasterizerState *>(state);
    const GPURasterizerStateDesc &desc = state->desc();

    if (desc.cullMode != CullMode::kDisabled) {
        this->state.enableCullFace(true);
        this->state.setCullFace(glState->cullMode);
    } else {
        this->state.enableCullFace(false);
    }

    this->state.enableDepthClamp(desc.depthClamp);
}

/** Initialize a GL sampler state object.
 * @param desc          Descriptor for sampler state. */
GLSamplerState::GLSamplerState(const GPUSamplerStateDesc &desc) :
    GPUSamplerState(desc)
{
    glGenSamplers(1, &m_sampler);

    /* Set wrap parameters. */
    GLint wrapS = GLUtil::convertSamplerAddressMode(m_desc.addressU);
    glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, wrapS);
    GLint wrapT = GLUtil::convertSamplerAddressMode(m_desc.addressV);
    glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, wrapT);
    GLint wrapR = GLUtil::convertSamplerAddressMode(m_desc.addressW);
    glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_R, wrapR);

    /* Set filtering mode. */
    switch (m_desc.filterMode) {
        case SamplerFilterMode::kBilinear:
            glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerFilterMode::kTrilinear:
            glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerFilterMode::kAnisotropic:
            glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            /* Set maximum anisotropy. TODO: global default if set to 0. In
             * this case we should insert the object into the hash table with
             * 0 replaced with setting, so we don't dup the same object. */
            glSamplerParameterf(m_sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, glm::clamp(
                static_cast<float>(m_desc.maxAnisotropy),
                1.0f,
                g_opengl->features.maxAnisotropy));

            break;
        default:
            glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
    }
}

/** Destroy the sampler state object. */
GLSamplerState::~GLSamplerState() {
    // TODO: If ever sampler states are destroyed at a time other than engine
    // shut down, we should add an equivalent of invalidateTexture() for the
    // sampler. Haven't done this now because it causes problems (sampler gets
    // destroyed after GLState is destroyed, segfault ensues).
    glDeleteSamplers(1, &m_sampler);
}

/** Bind the sampler to a texture unit.
 * @param index         Texture unit index to bind to. */
void GLSamplerState::bind(unsigned index) {
    g_opengl->state.bindSampler(index, m_sampler);
}

/** Create a sampler state object.
 * @param desc          Descriptor for sampler state.
 * @return              Pointer to created sampler state object. */
GPUSamplerStatePtr GLGPUInterface::createSamplerState(const GPUSamplerStateDesc &desc) {
    auto ret = m_samplerStates.find(desc);
    if (ret != m_samplerStates.end())
        return ret->second;

    GPUSamplerStatePtr state = new GLSamplerState(desc);
    m_samplerStates.insert(std::make_pair(desc, state));
    return state;
}
