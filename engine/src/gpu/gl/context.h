/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL context structure.
 */

#ifndef ORION_GPU_GL_CONTEXT_H
#define ORION_GPU_GL_CONTEXT_H

#include "state.h"

#include "core/engine.h"

/** Global OpenGL state. */
struct GLContext {
	SDL_GLContext sdl_context;	/**< SDL GL context. */
	SDL_Window *sdl_window;		/**< SDL main window. */
	GLuint default_vao;		/**< Default VAO when no object-specific VAO is in use. */
	GLState state;			/**< Cached GL state. */
};

extern GLContext *g_gl_context;

#endif /* ORION_GPU_GL_CONTEXT_H */
