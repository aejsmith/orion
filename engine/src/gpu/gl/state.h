/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL state management.
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

		/** @return		Current binding for the specified target. */
		GLuint &operator [](GLenum target) {
			switch(target) {
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
		TextureUnit() : target(GL_NONE), texture(0), sampler(0) {}
	};
public:
	int swapInterval;		/**< Current swap interval. */

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

	/** Object bindings. */
	GLuint boundVertexArray;
	BufferBindings boundBuffers;
	GLuint boundPipeline;
	unsigned activeTexture;
	TextureUnit *textureUnits;
public:
	GLState();
	~GLState();

	void initResources(GLFeatures &features);

	void setSwapInterval(int interval);

	void setClearColour(const glm::vec4 &colour);
	void setClearDepth(float depth);
	void setClearStencil(uint32_t stencil);

	void enableBlend(bool enable);
	void setBlendEquation(GLenum equation);
	void setBlendFunc(GLenum sourceFactor, GLenum destFactor);

	void enableDepthTest(bool enable);
	void enableDepthWrite(bool enable);
	void setDepthFunc(GLenum func);

	void bindVertexArray(GLuint array);
	void bindBuffer(GLenum target, GLuint buffer);
	void bindBufferBase(GLenum target, GLuint index, GLuint buffer);
	void bindPipeline(GLuint pipeline);
	void bindTexture(unsigned unit, GLenum target, GLuint texture);
	void bindSampler(unsigned unit, GLuint sampler);
};

/**
 * GPU state objects.
 */

/** OpenGL sampler state object implementation. */
class GLSamplerState : public GPUSamplerState {
public:
	GLSamplerState(const GPUSamplerStateDesc &desc);
	~GLSamplerState();

	void bind(unsigned index);
private:
	GLuint m_sampler;		/**< Sampler object. */
};
