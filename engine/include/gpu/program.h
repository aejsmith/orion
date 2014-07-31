/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GPU program class.
 *
 * @todo		Once the resource system is implemented, GPUProgram
 *			will become a resource object, and rather than being
 *			created through GPUInterface it will be loaded by a
 *			"loader" class. Loaders will be provided by the
 *			graphics API implementation, so for example asking for
 *			a .glsl resource file to be loaded would be handled by
 *			the GLSL loader and return a GLGPUProgram.
 */

#ifndef ORION_GPU_PROGRAM_H
#define ORION_GPU_PROGRAM_H

#include "core/defs.h"

#include <memory>

/** GPU program class. */
class GPUProgram : Noncopyable {
public:
	/** Type of the program. */
	enum Type {
		kVertexProgram,		/**< Vertex program. */
		kFragmentProgram,	/**< Fragment/pixel program. */
		kNumProgramTypes,
	};
public:
	virtual ~GPUProgram() {}

	/**
	 * Bind a uniform block in the program.
	 *
	 * Binds the uniform block with the specified name in the program to
	 * a uniform buffer binding point.
	 *
	 * @todo		This isn't great. It's only here to compensate
	 *			for the lack of GLSL layout modifier support on
	 *			OS X (fuck you Apple, update your OpenGL
	 *			implementation!). If they ever get round to
	 *			updating it, this can go. Same applies for
	 *			bind_texture().
	 *
	 * @param name		Name of uniform block.
	 * @param index		Uniform buffer binding point index.
	 */
	virtual void bind_uniforms(const char *name, unsigned index) = 0;

	/**
	 * Bind a sampler in the program.
	 *
	 * Binds the sampler with the specified name in the program to a
	 * texture unit.
	 *
	 * @param name		Name of sampler.
	 * @param index		Texture unit index.
	 */
	virtual void bind_texture(const char *name, unsigned index) = 0;

	/** @return		Type of the program. */
	Type type() const { return m_type; }
protected:
	/** Initialize the program.
	 * @param type		Type of the program. */
	explicit GPUProgram(Type type) : m_type(type) {}
private:
	Type m_type;			/**< Type of the program. */
};

/** Type of a GPU program pointer. */
typedef std::shared_ptr<GPUProgram> GPUProgramPtr;

#endif /* ORION_GPU_PROGRAM_H */
