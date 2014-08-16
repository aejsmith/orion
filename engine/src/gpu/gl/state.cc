/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL state management.
 */

#include "context.h"
#include "state.h"

/**
 * Initalize the GL state.
 *
 * This initializes the state object with default OpenGL state. Check the OpenGL
 * specifications to determine the correct default values when adding new
 * entries here.
 */
GLState::GLState() :
	swap_interval(0),
	clear_colour(0.0f, 0.0f, 0.0f, 0.0f),
	clear_depth(1.0f),
	clear_stencil(0.0f),
	blend_enabled(false),
	blend_equation(GL_FUNC_ADD),
	blend_source_factor(GL_ONE),
	blend_dest_factor(GL_ZERO),
	depth_test_enabled(false),
	depth_write_enabled(true),
	depth_func(GL_LESS),
	bound_vao(GL_NONE),
	bound_pipeline(GL_NONE)
{}

/** Set the current swap interval.
 * @param interval	Interval to set (passed to SDL_GL_SetSwapInterval). */
void GLState::set_swap_interval(int interval) {
	if(interval != this->swap_interval) {
		SDL_GL_SetSwapInterval(interval);
		this->swap_interval = interval;
	}
}

/** Set the colour clear value.
 * @param colour	Colour clear value. */
void GLState::set_clear_colour(const glm::vec4 &colour) {
	if(colour != this->clear_colour) {
		glClearColor(colour.r, colour.g, colour.b, colour.a);
		this->clear_colour = colour;
	}
}

/** Set the depth clear value.
 * @param depth		Depth clear value. */
void GLState::set_clear_depth(float depth) {
	if(depth != this->clear_depth) {
		glClearDepth(depth);
		this->clear_depth = depth;
	}
}

/** Set the stencil clear value.
 * @param stencil	Stencil clear value. */
void GLState::set_clear_stencil(uint32_t stencil) {
	if(stencil != this->clear_stencil) {
		glClearStencil(stencil);
		this->clear_stencil = stencil;
	}
}

/** Set whether blending is enabled.
 * @param enable	Whether to enable blending. */
void GLState::enable_blend(bool enable) {
	if(enable != this->blend_enabled) {
		if(enable) {
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}

		this->blend_enabled = enable;
	}
}

/** Set the blend equation.
 * @param equation	Blending equation. */
void GLState::set_blend_equation(GLenum equation) {
	if(equation != this->blend_equation) {
		glBlendEquation(equation);
		this->blend_equation = equation;
	}
}

/** Set the blending factors.
 * @param source_factor	Source factor.
 * @param dest_factor	Destination factor. */
void GLState::set_blend_func(GLenum source_factor, GLenum dest_factor) {
	if(source_factor != this->blend_source_factor || dest_factor != this->blend_dest_factor) {
		glBlendFunc(source_factor, dest_factor);
		this->blend_source_factor = source_factor;
		this->blend_dest_factor = dest_factor;
	}
}

/** Set whether the depth test is enabled.
 * @param enable	Whether to enable the depth test. */
void GLState::enable_depth_test(bool enable) {
	if(enable != this->depth_test_enabled) {
		if(enable) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}

		this->depth_test_enabled = enable;
	}
}

/** Set whether depth buffer writes are enabled.
 * @param enable	Whether to enable depth buffer writes. */
void GLState::enable_depth_write(bool enable) {
	if(enable != this->depth_write_enabled) {
		glDepthMask(enable);
		this->depth_write_enabled = enable;
	}
}

/** Set the depth comparison function.
 * @param func		Depth comparison function. */
void GLState::set_depth_func(GLenum func) {
	if(func != this->depth_func) {
		glDepthFunc(func);
		this->depth_func = func;
	}
}

/** Bind a VAO.
 * @param vao		VAO to bind. */
void GLState::bind_vao(GLuint vao) {
	if(vao != this->bound_vao) {
		glBindVertexArray(vao);
		this->bound_vao = vao;
	}
}

/** Bind a buffer.
 * @param target	Target to bind buffer to.
 * @param buffer	Buffer to bind. */
void GLState::bind_buffer(GLenum target, GLuint buffer) {
	if(target == GL_ELEMENT_ARRAY_BUFFER) {
		/* Since the element array buffer binding is part of VAO state,
		 * make sure we are on the default VAO. All element array buffer
		 * bindings done outside of VertexData::bind() should be done
		 * on the default VAO so that we don't affect the per-object
		 * VAOs and so that we can keep track of the currently bound
		 * buffer more easily. */
		bind_vao(g_gl_context->default_vao);
	}

	if(this->bound_buffers[target] != buffer) {
		glBindBuffer(target, buffer);
		this->bound_buffers[target] = buffer;
	}
}

/** Bind a buffer to an indexed buffer target.
 * @param target	Target to bind buffer to.
 * @param index		Binding point index.
 * @param buffer	Buffer to bind. */
void GLState::bind_buffer_base(GLenum target, GLuint index, GLuint buffer) {
	/* Brain damaged API design alert! glBindBufferBase also binds to the
	 * generic buffer binding point. */
	// TODO: Cache these
	glBindBufferBase(target, index, buffer);
	this->bound_buffers[target] = buffer;
}

/** Bind a program pipeline.
 * @param pipeline	Pipeline object to bind. */
void GLState::bind_pipeline(GLuint pipeline) {
	if(this->bound_pipeline != pipeline) {
		glBindProgramPipeline(pipeline);
		this->bound_pipeline = pipeline;
	}
}
