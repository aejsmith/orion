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
 * This initializes the state object with default OpenGL state. Be careful when
 * adding things here, check the OpenGL specifications to determine the correct
 * default values.
 */
GLState::GLState() :
	clear_colour(0.0f, 0.0f, 0.0f, 0.0f),
	clear_depth(1.0f),
	clear_stencil(0.0f),
	bound_vao(GL_NONE),
	bound_pipeline(GL_NONE)
{}

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