/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL state management.
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
	boundVertexArray(0),
	boundPipeline(0),
	activeTexture(0),
	textureUnits(nullptr)
{}

/** Destroy the GL state. */
GLState::~GLState() {
	if(this->textureUnits)
		delete[] this->textureUnits;
}

/** Allocate arrays dependent on GL implementation capabilities.
 * @param features	GL features description. */
void GLState::initResources(GLFeatures &features) {
	this->textureUnits = new TextureUnit[features.maxTextureUnits];
}

/** Set the current swap interval.
 * @param interval	Interval to set (passed to SDL_GL_SetSwapInterval). */
void GLState::setSwapInterval(int interval) {
	if(interval != this->swapInterval) {
		SDL_GL_SetSwapInterval(interval);
		this->swapInterval = interval;
	}
}

/** Set the colour clear value.
 * @param colour	Colour clear value. */
void GLState::setClearColour(const glm::vec4 &colour) {
	if(colour != this->clearColour) {
		glClearColor(colour.r, colour.g, colour.b, colour.a);
		this->clearColour = colour;
	}
}

/** Set the depth clear value.
 * @param depth		Depth clear value. */
void GLState::setClearDepth(float depth) {
	if(depth != this->clearDepth) {
		glClearDepth(depth);
		this->clearDepth = depth;
	}
}

/** Set the stencil clear value.
 * @param stencil	Stencil clear value. */
void GLState::setClearStencil(uint32_t stencil) {
	if(stencil != this->clearStencil) {
		glClearStencil(stencil);
		this->clearStencil = stencil;
	}
}

/** Set whether blending is enabled.
 * @param enable	Whether to enable blending. */
void GLState::enableBlend(bool enable) {
	if(enable != this->blendEnabled) {
		if(enable) {
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}

		this->blendEnabled = enable;
	}
}

/** Set the blend equation.
 * @param equation	Blending equation. */
void GLState::setBlendEquation(GLenum equation) {
	if(equation != this->blendEquation) {
		glBlendEquation(equation);
		this->blendEquation = equation;
	}
}

/** Set the blending factors.
 * @param sourceFactor	Source factor.
 * @param destFactor	Destination factor. */
void GLState::setBlendFunc(GLenum sourceFactor, GLenum destFactor) {
	if(sourceFactor != this->blendSourceFactor || destFactor != this->blendDestFactor) {
		glBlendFunc(sourceFactor, destFactor);
		this->blendSourceFactor = sourceFactor;
		this->blendDestFactor = destFactor;
	}
}

/** Set whether the depth test is enabled.
 * @param enable	Whether to enable the depth test. */
void GLState::enableDepthTest(bool enable) {
	if(enable != this->depthTestEnabled) {
		if(enable) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}

		this->depthTestEnabled = enable;
	}
}

/** Set whether depth buffer writes are enabled.
 * @param enable	Whether to enable depth buffer writes. */
void GLState::enableDepthWrite(bool enable) {
	if(enable != this->depthWriteEnabled) {
		glDepthMask(enable);
		this->depthWriteEnabled = enable;
	}
}

/** Set the depth comparison function.
 * @param func		Depth comparison function. */
void GLState::setDepthFunc(GLenum func) {
	if(func != this->depthFunc) {
		glDepthFunc(func);
		this->depthFunc = func;
	}
}

/** Bind a VAO.
 * @param array		VAO to bind. */
void GLState::bindVertexArray(GLuint array) {
	if(array != this->boundVertexArray) {
		glBindVertexArray(array);
		this->boundVertexArray = array;
	}
}

/** Bind a buffer.
 * @param target	Target to bind buffer to.
 * @param buffer	Buffer to bind. */
void GLState::bindBuffer(GLenum target, GLuint buffer) {
	if(target == GL_ELEMENT_ARRAY_BUFFER) {
		/* Since the element array buffer binding is part of VAO state,
		 * make sure we are on the default VAO. All element array buffer
		 * bindings done outside of GLVertexData::bind() should be done
		 * on the default VAO so that we don't affect the per-object
		 * VAOs and so that we can keep track of the currently bound
		 * buffer more easily. */
		bindVertexArray(g_opengl->defaultVertexArray);
	}

	if(this->boundBuffers[target] != buffer) {
		glBindBuffer(target, buffer);
		this->boundBuffers[target] = buffer;
	}
}

/** Bind a buffer to an indexed buffer target.
 * @param target	Target to bind buffer to.
 * @param index		Binding point index.
 * @param buffer	Buffer to bind. */
void GLState::bindBufferBase(GLenum target, GLuint index, GLuint buffer) {
	/* Brain damaged API design alert! glBindBufferBase also binds to the
	 * generic buffer binding point. */
	// TODO: Cache these
	glBindBufferBase(target, index, buffer);
	this->boundBuffers[target] = buffer;
}

/** Bind a program pipeline.
 * @param pipeline	Pipeline object to bind. */
void GLState::bindPipeline(GLuint pipeline) {
	if(this->boundPipeline != pipeline) {
		glBindProgramPipeline(pipeline);
		this->boundPipeline = pipeline;
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
 * @param unit		Texture unit to bind to. Note this is specified as
 *			the unit, not as a GL_TEXTUREn constant - this is added
 *			on automatically.
 * @param target	Texture target to bind.
 * @param texture	Texture object to bind.
 */
void GLState::bindTexture(unsigned unit, GLenum target, GLuint texture) {
	if(this->activeTexture != unit) {
		glActiveTexture(GL_TEXTURE0 + unit);
		this->activeTexture = unit;
	}

	TextureUnit &unitState = this->textureUnits[unit];
	if(unitState.target != target || unitState.texture != texture) {
		if(unitState.target != target && unitState.texture != 0) {
			/* Unbind the texture currently bound so that we don't
			 * have multiple textures bound to different targets. */
			glBindTexture(unitState.target, 0);
		}

		glBindTexture(target, texture);
		unitState.target = target;
		unitState.texture = texture;
	}
}

/** Bind a sampler to a texture unit.
 * @param unit		Texture unit to bind to. Note this is specified as
 *			the unit, not as a GL_TEXTUREn constant.
 * @param sampler	Texture sampler to bind. */
void GLState::bindSampler(unsigned unit, GLuint sampler) {
	TextureUnit &unitState = this->textureUnits[unit];
	if(unitState.sampler != sampler) {
		glBindSampler(unit, sampler);
		unitState.sampler = sampler;
	}
}

/**
 * State object management.
 */

/** Initialize a GL sampler state object.
 * @param desc		Descriptor for sampler state. */
GLSamplerState::GLSamplerState(const GPUSamplerStateDesc &desc) :
	GPUSamplerState(desc)
{
	glGenSamplers(1, &m_sampler);

	/* Set wrap parameters. */
	GLint wrapS = gl::convertSamplerAddressMode(m_desc.addressU);
	glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, wrapS);
	GLint wrapT = gl::convertSamplerAddressMode(m_desc.addressV);
	glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, wrapT);
	GLint wrapR = gl::convertSamplerAddressMode(m_desc.addressW);
	glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_R, wrapR);

	/* Set filtering mode. */
	switch(m_desc.filterMode) {
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

		/* Set maximum anisotropy. TODO: global default if set to 0. */
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
	glDeleteSamplers(1, &m_sampler);
}

/** Bind the sampler to a texture unit.
 * @param index		Texture unit index to bind to. */
void GLSamplerState::bind(unsigned index) {
	g_opengl->state.bindSampler(index, m_sampler);
}

/** Create a sampler state object.
 * @param desc		Descriptor for sampler state.
 * @return		Pointer to created sampler state object. */
GPUSamplerStatePtr GLGPUInterface::createSamplerState(const GPUSamplerStateDesc &desc) {
	auto ret = m_samplerStates.find(desc);
	if(ret != m_samplerStates.end())
		return ret->second;

	GPUSamplerStatePtr state = new GLSamplerState(desc);
	m_samplerStates.insert(std::make_pair(desc, state));
	return state;
}
