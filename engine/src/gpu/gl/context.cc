/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GL context management.
 */

#include "context.h"
#include "gpu.h"

#include "lib/utility.h"

/** Global GL context. */
GLContext *g_gl_context = nullptr;

/** Required OpenGL features. */
static const char *required_gl_features[] = {
	"GL_VERSION_3_3",
	"GL_ARB_separate_shader_objects",
};

/** GL version to use. */
static const int kGLMajorVersion = 3;
static const int kGLMinorVersion = 3;

#if ORION_GL_DEBUG

/** GL debug output callback.
 * @param source	Message source.
 * @param type		Message type.
 * @param id		Implementation-dependent ID.
 * @param severity	Severity of the message.
 * @param length	Length of the message.
 * @param message	Message text.
 * @param param		User-defined parameter (unused). */
static GLEWAPIENTRY void gl_debug_callback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	GLvoid *param)
{
	const char *source_str = "OTHER";
	switch(source) {
	case GL_DEBUG_SOURCE_API:
		source_str = "API";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		source_str = "SHADER_COMPILER";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		source_str = "WINDOW_SYSTEM";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		source_str = "THIRD_PARTY";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		source_str = "APPLICATION";
		break;
	}

	const char *type_str = "OTHER";
	switch(type) {
	case GL_DEBUG_TYPE_ERROR:
		type_str = "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		type_str = "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		type_str = "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		type_str = "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		type_str = "PORTABILITY";
		break;
	}

	LogLevel level = LogLevel::kDebug;
	switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		level = LogLevel::kError;
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
	case GL_DEBUG_SEVERITY_LOW:
		level = LogLevel::kWarning;
		break;
	}

	orion_log(level, "GL [source = %s, type = %s]:", source_str, type_str);
	orion_log(level, "%s", message);

	if(severity == GL_DEBUG_SEVERITY_HIGH)
		orion_abort("GL driver error (see log for details)");
}

#endif /* ORION_GL_DEBUG */

/** Initialize the GPU interface. */
GLGPUInterface::GLGPUInterface() {
	/* Prior to window creation we must set SDL attributes for OpenGL. */
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	#ifdef __APPLE__
		/* On OS X, we want to create a Core profile context. If we do
		 * not, we get a legacy profile which only supports GL 2.1.
		 * However, on other systems, use a compatibility profile.
		 * Creating a core profile tends to give a context which only
		 * supports the specific GL version requested (even though the
		 * GLX spec, for example, permits later versions to be
		 * returned). A compatibility profile on the other hand will
		 * always support the latest version supported by the driver.
		 * In fact, NVIDIA recommend that you use a compatibility
		 * profile instead of core profile. */
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, kGLMajorVersion);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, kGLMinorVersion);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	#endif

	#if ORION_GL_DEBUG
		/* If GL debugging is enabled, enable the debug context flag so
		 * that we can use ARB_debug_output. */
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	#endif
}

/** Initialize the GPU interface.
 * @param window	Created SDL window. */
void GLGPUInterface::init(SDL_Window *window) {
	g_gl_context = new GLContext;
	g_gl_context->sdl_window = window;

	g_gl_context->sdl_context = SDL_GL_CreateContext(window);
	if(!g_gl_context->sdl_context)
		orion_abort("Failed to create GL context: %s", SDL_GetError());

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

	#if ORION_GL_DEBUG
		/* Hook up debug output if supported. */
		if(glewIsSupported("GL_ARB_debug_output")) {
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(gl_debug_callback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
				GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
				ORION_GL_DEBUG_NOTIFICATIONS);
		}
	#endif

	/* Create the default VAO. */
	glGenVertexArrays(1, &g_gl_context->default_vao);
	g_gl_context->state.bind_vao(g_gl_context->default_vao);
}

/** Shut down the GPU interface. */
GLGPUInterface::~GLGPUInterface() {
	SDL_GL_DeleteContext(g_gl_context->sdl_context);
	delete g_gl_context;
}
