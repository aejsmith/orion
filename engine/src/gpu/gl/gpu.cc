/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL GPU interface implementation.
 */

#include "buffer.h"
#include "context.h"
#include "gpu.h"
#include "pipeline.h"
#include "program.h"
#include "vertex_data.h"

#include "lib/utility.h"

/** Global GL context. */
GLContext *g_gl_context = nullptr;

/** Required OpenGL features. */
static const char *required_gl_features[] = {
	"GL_VERSION_4_1",
};

/** Initialize the GPU interface.
 * @param config	Engine configuration. */
GLGPUInterface::GLGPUInterface(const EngineConfiguration &config) {
	g_gl_context = new GLContext;

	/* Set SDL attributes for OpenGL. */
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	/* On OS X, we want to create a Core profile context. If we do not,
	 * we get a legacy profile which only supports GL 2.1. However, on
	 * other systems, use a compatibility profile. Creating a core profile
	 * tends to give a context which only supports the specific GL version
	 * requested (even though the GLX spec, for example, permits later
	 * versions to be returned). A compatibility profile on the other hand
	 * will always support the latest version supported by the driver. In
	 * fact, NVIDIA recommend that you use a compatibility profile instead
	 * of core profile. */
	#ifdef __APPLE__
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	#endif

	uint32_t flags = SDL_WINDOW_OPENGL;
	if(config.display_fullscreen)
		flags |= SDL_WINDOW_FULLSCREEN;

	g_gl_context->sdl_window = SDL_CreateWindow(
		config.title.c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		config.display_width, config.display_height,
		flags);
	if(!g_gl_context->sdl_window)
		orion_abort("Failed to create main window: %s", SDL_GetError());

	g_gl_context->sdl_context = SDL_GL_CreateContext(g_gl_context->sdl_window);
	if(!g_gl_context->sdl_context)
		orion_abort("Failed to create GL context: %s", SDL_GetError());

	SDL_GL_SetSwapInterval(config.display_vsync);

	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
		orion_abort("Failed to initialize GLEW");

	/* Log some OpenGL details. */
	orion_log(LogLevel::kInfo, "OpenGL vendor:   %s", glGetString(GL_VENDOR));
	orion_log(LogLevel::kInfo, "OpenGL renderer: %s", glGetString(GL_RENDERER));
	orion_log(LogLevel::kInfo, "OpenGL version:  %s", glGetString(GL_VERSION));
	orion_log(LogLevel::kInfo, "GLEW version:    %s", glewGetString(GLEW_VERSION));

	/* Check for required OpenGL functionality. */
	for(size_t i = 0; i < util::array_size(required_gl_features); i++) {
		if(!glewIsSupported(required_gl_features[i])) {
			orion_abort(
				"Required OpenGL feature `%s' is not supported",
				required_gl_features[i]);
		}
	}

	/* Create the default VAO. */
	glGenVertexArrays(1, &g_gl_context->default_vao);
	glBindVertexArray(g_gl_context->default_vao);
}

/** Shut down the GPU interface. */
GLGPUInterface::~GLGPUInterface() {
	SDL_GL_DeleteContext(g_gl_context->sdl_context);
	SDL_DestroyWindow(g_gl_context->sdl_window);

	delete g_gl_context;
}

/** Create a GPU buffer.
 * @see		GPUBuffer::GPUBuffer().
 * @return	Pointer to created vertex buffer. */
GPUBufferPtr GLGPUInterface::create_buffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) {
	GPUBuffer *buffer = new GLBuffer(type, usage, size);
	return GPUBufferPtr(buffer);
}

/** Create a vertex data object.
 * @see			VertexData::VertexData().
 * @return		Pointer to created vertex data object. */
VertexDataPtr GLGPUInterface::create_vertex_data(size_t vertices) {
	VertexData *data = new GLVertexData(vertices);
	return VertexDataPtr(data);
}

/** Create a pipeline object.
 * @return		Pointer to created pipeline. */
GPUPipelinePtr GLGPUInterface::create_pipeline() {
	GPUPipeline *pipeline = new GLPipeline();
	return GPUPipelinePtr(pipeline);
}

/** Load a GPU program.
 * @param path		Path to the program source.
 * @param type		Type of the program.
 * @return		Pointer to created program. */
GPUProgramPtr GLGPUInterface::load_program(const char *path, GPUProgram::Type type) {
	GPUProgram *program = new GLProgram(path, type);
	return GPUProgramPtr(program);
}

/** Swap buffers. */
void GLGPUInterface::swap_buffers() {
	SDL_GL_SwapWindow(g_gl_context->sdl_window);
}

/** Clear rendering buffers.
 * @param buffers	Buffers to clear (bitmask of RenderBuffer values).
 * @param colour	Colour to clear to.
 * @param depth		Depth value to clear to.
 * @param stencil	Stencil value to clear to. */
void GLGPUInterface::clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil) {
	GLbitfield mask = 0;

	if(buffers & RenderBuffer::kColourBuffer) {
		glClearColor(colour.r, colour.g, colour.b, colour.a);
		mask |= GL_COLOR_BUFFER_BIT;
	}

	if(buffers & RenderBuffer::kDepthBuffer) {
		glClearDepth(depth);
		mask |= GL_DEPTH_BUFFER_BIT;
	}

	if(buffers & RenderBuffer::kStencilBuffer) {
		glClearStencil(stencil);
		mask |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(mask);
}

/** Bind a pipeline for rendering.
 * @param _pipeline	Pipeline to use. */
void GLGPUInterface::bind_pipeline(const GPUPipelinePtr &_pipeline) {
	GLPipeline *pipeline = static_cast<GLPipeline *>(_pipeline.get());
	pipeline->bind();
}

/** Draw primitives.
 * @param type		Primitive type to render.
 * @param _vertices	Vertex data to use.
 * @param indices	Index data to use (can be null). */
void GLGPUInterface::draw(PrimitiveType type, const VertexDataPtr &_vertices, const IndexDataPtr &indices) {
	GLVertexData *vertices = static_cast<GLVertexData *>(_vertices.get());

	/* Bind the VAO and the index buffer (if any). */
	vertices->bind((indices) ? indices->buffer() : nullptr);

	GLenum mode = gl::convert_primitive_type(type);
	if(indices) {
		/* FIXME: Check whether index type is supported (in generic
		 * code?) */
		glDrawElements(mode, indices->count(), gl::convert_index_type(indices->type()), nullptr);
	} else {
		glDrawArrays(mode, 0, vertices->count());
	}

	/* Restore default VAO. */
	glBindVertexArray(g_gl_context->default_vao);
}
