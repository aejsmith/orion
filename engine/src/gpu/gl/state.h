/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL state management.
 */

#ifndef ORION_GPU_GL_STATE_H
#define ORION_GPU_GL_STATE_H

#include "defs.h"

struct GLContext;

/**
 * Object managing OpenGL state.
 *
 * This class holds information about OpenGL state. GLContext contains a state
 * object which caches the current state for that context. In most cases, GL
 * state changes should be made by calling functions on the active context's
 * state object.
 *
 * When adding state to this structure, be sure to add default initializers to
 * the constructor.
 */
struct GLState {
	/** Class holding buffer bindings. */
	class BufferBindings {
	public:
		BufferBindings() :
			m_array_buffer(GL_NONE),
			m_element_array_buffer(GL_NONE),
			m_uniform_buffer(GL_NONE)
		{}

		GLuint &operator [](GLenum target) {
			switch(target) {
			case GL_ARRAY_BUFFER:		return m_array_buffer;
			case GL_ELEMENT_ARRAY_BUFFER:	return m_element_array_buffer;
			case GL_UNIFORM_BUFFER:		return m_uniform_buffer;
			default:			unreachable();
			}
		}
	private:
		GLuint m_array_buffer;
		GLuint m_element_array_buffer;
		GLuint m_uniform_buffer;
	};
public:
	int swap_interval;		/**< Current swap interval. */

	/** Clear state. */
	glm::vec4 clear_colour;
	float clear_depth;
	uint32_t clear_stencil;

	/** Object bindings. */
	GLuint bound_vao;
	BufferBindings bound_buffers;
	GLuint bound_pipeline;
public:
	GLState();

	void set_swap_interval(int interval);

	void set_clear_colour(const glm::vec4 &colour);
	void set_clear_depth(float depth);
	void set_clear_stencil(uint32_t stencil);

	void bind_vao(GLuint vao);
	void bind_buffer(GLenum target, GLuint buffer);
	void bind_buffer_base(GLenum target, GLuint index, GLuint buffer);
	void bind_pipeline(GLuint pipeline);
};

#endif /* ORION_GPU_GL_STATE_H */
