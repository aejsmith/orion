/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL state management.
 */

#ifndef ORION_GPU_GL_STATE_H
#define ORION_GPU_GL_STATE_H

#include "core/defs.h"

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
			m_array_buffer(0),
			m_element_array_buffer(0),
			m_uniform_buffer(0)
		{}

		/** @return		Current binding for the specified target. */
		GLuint &operator [](GLenum target) {
			switch(target) {
			case GL_ARRAY_BUFFER:
				return m_array_buffer;
			case GL_ELEMENT_ARRAY_BUFFER:
				return m_element_array_buffer;
			case GL_UNIFORM_BUFFER:
				return m_uniform_buffer;
			default:
				unreachable();
			}
		}
	private:
		GLuint m_array_buffer;
		GLuint m_element_array_buffer;
		GLuint m_uniform_buffer;
	};

	/** Structure holding texture unit state. */
	struct TextureUnit {
		GLenum target;
		GLuint texture;
	public:
		TextureUnit() : target(GL_NONE), texture(0) {}
	};
public:
	int swap_interval;		/**< Current swap interval. */

	/** Clear state. */
	glm::vec4 clear_colour;
	float clear_depth;
	uint32_t clear_stencil;

	/** Blending state. */
	bool blend_enabled;
	GLenum blend_equation;
	GLenum blend_source_factor;
	GLenum blend_dest_factor;

	/** Depth testing state. */
	bool depth_test_enabled;
	bool depth_write_enabled;
	GLenum depth_func;

	/** Object bindings. */
	GLuint bound_vao;
	BufferBindings bound_buffers;
	GLuint bound_pipeline;
	unsigned active_texture;
	TextureUnit *texture_units;
public:
	GLState();
	~GLState();

	void init_resources(GLFeatures &features);

	void set_swap_interval(int interval);

	void set_clear_colour(const glm::vec4 &colour);
	void set_clear_depth(float depth);
	void set_clear_stencil(uint32_t stencil);

	void enable_blend(bool enable);
	void set_blend_equation(GLenum equation);
	void set_blend_func(GLenum source_factor, GLenum dest_factor);

	void enable_depth_test(bool enable);
	void enable_depth_write(bool enable);
	void set_depth_func(GLenum func);

	void bind_vao(GLuint vao);
	void bind_buffer(GLenum target, GLuint buffer);
	void bind_buffer_base(GLenum target, GLuint index, GLuint buffer);
	void bind_pipeline(GLuint pipeline);
	void bind_texture(unsigned unit, GLenum target, GLuint texture);
};

#endif /* ORION_GPU_GL_STATE_H */
