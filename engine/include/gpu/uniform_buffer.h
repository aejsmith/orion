/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Uniform buffer helper classes.
 */

#ifndef ORION_GPU_UNIFORM_BUFFER_H
#define ORION_GPU_UNIFORM_BUFFER_H

#include "gpu/gpu.h"

/**
 * Class maintaining a dynamically updated uniform buffer.
 *
 * This is a helper class for an object which contains a dynamically updated
 * uniform buffer. It only updates the uniform buffer when it is actually
 * needed. Whenever the data in the uniform buffer becomes outdated, the derived
 * class should call invalidate(), and the next time get() is called the buffer
 * will be updated.
 *
 * @tparam Uniforms	Type of the uniform structure.
 */
template <typename Uniforms>
class DynamicUniformBuffer {
public:
	DynamicUniformBuffer() : m_valid(false) {}

	/** Get uniform buffer for the object, updating if necessary.
	 * @param updater	Function to update the buffer if required.
	 *			The entire buffer is invalidated when this is
	 *			called so must be recreated from scratch.
	 * @return		Pointer to uniform buffer for the object. */
	template<typename Func> GPUBufferPtr get(Func func) {
		if(!m_valid) {
			/* Create the uniform buffer if it does not exist. */
			if(!m_buffer) {
				m_buffer = g_engine->gpu()->create_buffer(
					GPUBuffer::kUniformBuffer,
					GPUBuffer::kDynamicDrawUsage,
					sizeof(Uniforms));
			}

			GPUBufferMapper<Uniforms> uniforms(
				m_buffer,
				GPUBuffer::kMapInvalidate,
				GPUBuffer::kWriteAccess);

			func(uniforms);
			m_valid = true;
		}

		return m_buffer;
	}

	/** Mark the uniforms as invalid. */
	void invalidate() { m_valid = false; }
private:
	GPUBufferPtr m_buffer;		/**< Uniform buffer. */
	bool m_valid;			/**< Whether the buffer contents are valid. */
};

#endif /* ORION_GPU_UNIFORM_BUFFER_H */
