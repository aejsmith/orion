/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GPU shader class.
 */

#pragma once

#include "gpu/defs.h"

/** GPU shader class. */
class GPUShader : public GPUResource {
public:
	/** Type of the shader. */
	enum Type {
		kVertexShader,		/**< Vertex shader. */
		kFragmentShader,	/**< Fragment/pixel shader. */
		kNumShaderTypes,
	};
public:
	/**
	 * Bind a uniform block in the shader.
	 *
	 * Binds the uniform block with the specified name in the shader to
	 * a uniform buffer binding point.
	 *
	 * @todo		This isn't great. It's only here to compensate
	 *			for the lack of GLSL layout modifier support on
	 *			OS X (fuck you Apple, update your OpenGL
	 *			implementation!). If they ever get round to
	 *			updating it, this can go. Same applies for
	 *			bindTexture().
	 *
	 * @param name		Name of uniform block.
	 * @param index		Uniform buffer binding point index.
	 */
	virtual void bindUniforms(const char *name, unsigned index) = 0;

	/**
	 * Bind a sampler in the shader.
	 *
	 * Binds the sampler with the specified name in the shader to a
	 * texture unit.
	 *
	 * @param name		Name of sampler.
	 * @param index		Texture unit index.
	 */
	virtual void bindTexture(const char *name, unsigned index) = 0;

	/** @return		Type of the shader. */
	Type type() const { return m_type; }
protected:
	/** Initialize the shader.
	 * @param type		Type of the shader. */
	explicit GPUShader(Type type) : m_type(type) {}
private:
	Type m_type;			/**< Type of the shader. */
};

/** Type of a GPU shader pointer. */
typedef GPUResourcePtr<GPUShader> GPUShaderPtr;
