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
	SDL_Window *sdl_window;		/**< SDL window. */
	SDL_GLContext sdl_context;	/**< SDL GL context. */

	/** Default VAO set when no object-specific VAO is in use. */
	GLuint default_vao;

	/** Cached GL state. */
	GLState state;
public:
	GLContext(const EngineConfiguration &config);
	~GLContext();
};

extern GLContext *g_gl_context;

#endif /* ORION_GPU_GL_CONTEXT_H */
