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

	/** Structure describing a resource. */
	struct Resource {
		std::string name;	/**< Name of the resource. */
		unsigned index;		/**< Index of the resource for use with bind functions. */
	};

	/** Type of a resource list, given as a pair of name and index.
	 * @note		List rather than a vector to allow for sparse
	 *			indices. */
	typedef std::list<Resource> ResourceList;
public:
	/** @return		Type of the shader. */
	Type type() const { return m_type; }

	/** Query active uniform blocks in the program.
	 * @param list		Resource list to fill in. */
	virtual void queryUniformBlocks(ResourceList &list) = 0;

	/** Query active texture samplers in the program.
	 * @param list		Resource list to fill in. */
	virtual void querySamplers(ResourceList &list) = 0;

	/**
	 * Bind a uniform block in the shader.
	 *
	 * Specifies that the uniform block at the specified index (as returned
	 * from queryUniformBlocks()) should refer to the uniform buffer which
	 * is bound in the specified slot at the time of a draw call involving
	 * the shader.
	 *
	 * @param index		Index of uniform block.
	 * @param slot		Uniform buffer slot.
	 */
	virtual void bindUniformBlock(unsigned index, unsigned slot) = 0;

	/**
	 * Bind a texture sampler in the shader.
	 *
	 * Specifies that the texture sampler at the specified index (as
	 * returned from querySamplers()) should refer to the texture which is
	 * bound in the speicified slot at the time of a draw call involving the
	 * shader.
	 *
	 * @param index		Index of sampler.
	 * @param slot		Texture slot.
	 */
	virtual void bindSampler(unsigned index, unsigned slot) = 0;
protected:
	/** Initialize the shader.
	 * @param type		Type of the shader. */
	explicit GPUShader(Type type) : m_type(type) {}
private:
	Type m_type;			/**< Type of the shader. */
};

/** Type of a GPU shader pointer. */
typedef GPUResourcePtr<GPUShader> GPUShaderPtr;
