/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL GPU interface implementation.
 */

#include "gl.h"

#include "lib/utility.h"

/** Global GL GPU interface. */
GLGPUInterface *g_opengl = nullptr;

/** Target GL major version. */
static const int kGLMajorVersion = 3;

/** Target GL minor version. */
static const int kGLMinorVersion = 3;

/** Required OpenGL extensions. */
static const char *required_gl_extensions[] = {
	"GL_ARB_separate_shader_objects",
	"GL_ARB_texture_storage",
};

/** Initialize the GPU interface. */
GLGPUInterface::GLGPUInterface() :
	default_vao(0),
	m_sdl_context(nullptr)
{
	g_opengl = this;

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

/** Shut down the GPU interface. */
GLGPUInterface::~GLGPUInterface() {
	if(m_sdl_context)
		SDL_GL_DeleteContext(m_sdl_context);

	g_opengl = nullptr;
}

/** Initialize the GPU interface.
 * @param window	Created SDL window. */
void GLGPUInterface::init(SDL_Window *window) {
	m_sdl_context = SDL_GL_CreateContext(window);
	if(!m_sdl_context)
		orion_abort("Failed to create GL context: %s", SDL_GetError());

	SDL_GL_SetSwapInterval(0);

	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
		orion_abort("Failed to initialize GLEW");

	/* Initialize the features table and check requirements. */
	init_features();
	this->state.init_resources(this->features);

	/* Populate the pixel format table. */
	init_pixel_formats();

	#if ORION_GL_DEBUG
		/* Hook up debug output if supported. */
		if(this->features["GL_ARB_debug_output"]) {
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback((GLDEBUGPROC)debug_callback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
				GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
				ORION_GL_DEBUG_NOTIFICATIONS);
		}
	#endif

	/* Create the default VAO. */
	glGenVertexArrays(1, &this->default_vao);
	this->state.bind_vao(this->default_vao);

	/* Set up some default state. FIXME */
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

/** Detect GL features and check requirements. */
void GLGPUInterface::init_features() {
	GLFeatures &features = this->features;

	/* Log some OpenGL details. */
	orion_log(LogLevel::kInfo, "OpenGL vendor:   %s", glGetString(GL_VENDOR));
	orion_log(LogLevel::kInfo, "OpenGL renderer: %s", glGetString(GL_RENDERER));
	orion_log(LogLevel::kInfo, "OpenGL version:  %s", glGetString(GL_VERSION));

	/* Check whether the version number is high enough. */
	GLint major = 0, minor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	if(major < kGLMajorVersion || (major == kGLMajorVersion && minor < kGLMinorVersion))
		orion_abort("OpenGL version %d.%d is required", kGLMajorVersion, kGLMinorVersion);

	/* Query supported extensions. */
	GLint count = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &count);
	for(GLint i = 0; i < count; i++) {
		std::string extension((const char *)glGetStringi(GL_EXTENSIONS, i));
		features.extensions.insert(extension);
	}

	/* Print out a (sorted) list of the extensions found. */
	orion_log(LogLevel::kDebug, "OpenGL extensions:");
	for(const std::string &extension : features.extensions)
		orion_log(LogLevel::kDebug, " %s", extension.c_str());

	/* Check for required extensions. */
	for(size_t i = 0; i < util::array_size(required_gl_extensions); i++) {
		if(!this->features[required_gl_extensions[i]]) {
			orion_abort(
				"Required OpenGL extension '%s' is not supported",
				required_gl_extensions[i]);
		}
	}

	/* Cache some GL information. */
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &features.max_texture_units);
}

/** Initialize the supported pixel format conversion table. */
void GLGPUInterface::init_pixel_formats() {
	PixelFormatArray &f = this->pixel_formats;

	/* TODO: For now this is a static table. We should identify the formats
	 * that are actually supported, and have an engine generic table of
	 * supported formats.
	 * TODO: If we ever run on a big endian platform, the packed pixel
	 * formats will need to be changed. */
	f[PixelFormat::kRGBA8]		 = { GL_RGBA8,		   GL_RGBA,	       GL_UNSIGNED_INT_8_8_8_8_REV };
	f[PixelFormat::kRGB8]		 = { GL_RGB8,		   GL_RGB,	       GL_UNSIGNED_BYTE };
	f[PixelFormat::kRG8]		 = { GL_RG8,		   GL_RG,	       GL_UNSIGNED_BYTE };
	f[PixelFormat::kR8]		 = { GL_R8,		   GL_RED,	       GL_UNSIGNED_BYTE };
	f[PixelFormat::kRGBA16]		 = { GL_RGBA16,		   GL_RGBA,	       GL_UNSIGNED_SHORT };
	f[PixelFormat::kRGB16]		 = { GL_RGB16,		   GL_RGB,	       GL_UNSIGNED_SHORT };
	f[PixelFormat::kRG16]		 = { GL_RG16,		   GL_RG,	       GL_UNSIGNED_SHORT };
	f[PixelFormat::kR16]		 = { GL_R16,		   GL_RED,	       GL_UNSIGNED_SHORT };
	f[PixelFormat::kRGBA16Float]	 = { GL_RGBA16F,	   GL_RGBA,	       GL_HALF_FLOAT };
	f[PixelFormat::kRGB16Float]	 = { GL_RGB16F,		   GL_RGB,	       GL_HALF_FLOAT };
	f[PixelFormat::kRG16Float]	 = { GL_RG16F,		   GL_RG,	       GL_HALF_FLOAT };
	f[PixelFormat::kR16Float]	 = { GL_R16F,		   GL_RED,	       GL_HALF_FLOAT };
	f[PixelFormat::kRGBA32Float]	 = { GL_RGBA32F,	   GL_RGBA,	       GL_FLOAT };
	f[PixelFormat::kRGB32Float]	 = { GL_RGB32F,		   GL_RGB,	       GL_FLOAT };
	f[PixelFormat::kRG32Float]	 = { GL_RG32F,		   GL_RG,	       GL_FLOAT };
	f[PixelFormat::kR32Float]	 = { GL_R32F,		   GL_RED,	       GL_FLOAT };
	f[PixelFormat::kRGBA8Signed]	 = { GL_RGBA8I,		   GL_RGBA,	       GL_BYTE };
	f[PixelFormat::kRGB8Signed]	 = { GL_RGB8I,		   GL_RGB,	       GL_BYTE };
	f[PixelFormat::kRG8Signed]	 = { GL_RG8I,		   GL_RG,	       GL_BYTE };
	f[PixelFormat::kR8Signed]	 = { GL_R8I,		   GL_RED,	       GL_BYTE };
	f[PixelFormat::kRGBA16Signed]	 = { GL_RGBA16I,	   GL_RGBA,	       GL_SHORT };
	f[PixelFormat::kRGB16Signed]	 = { GL_RGB16I,		   GL_RGB,	       GL_SHORT };
	f[PixelFormat::kRG16Signed]	 = { GL_RG16I,		   GL_RG,	       GL_SHORT };
	f[PixelFormat::kR16Signed]	 = { GL_R16I,		   GL_RED,	       GL_SHORT };
	f[PixelFormat::kRGBA32Signed]	 = { GL_RGBA32I,	   GL_RGBA,	       GL_INT };
	f[PixelFormat::kRGB32Signed]	 = { GL_RGB32I,		   GL_RGB,	       GL_INT };
	f[PixelFormat::kRG32Signed]	 = { GL_RG32I,		   GL_RG,	       GL_INT };
	f[PixelFormat::kR32Signed]	 = { GL_R32I,		   GL_RED,	       GL_INT };
	f[PixelFormat::kRGBA8Unsigned]	 = { GL_RGBA8UI,	   GL_RGBA,	       GL_UNSIGNED_INT_8_8_8_8_REV };
	f[PixelFormat::kRGB8Unsigned]	 = { GL_RGB8UI,		   GL_RGB,	       GL_UNSIGNED_BYTE };
	f[PixelFormat::kRG8Unsigned]	 = { GL_RG8UI,		   GL_RG,	       GL_UNSIGNED_BYTE };
	f[PixelFormat::kR8Unsigned]	 = { GL_R8UI,		   GL_RED,	       GL_UNSIGNED_BYTE };
	f[PixelFormat::kRGBA16Unsigned]	 = { GL_RGBA16UI,	   GL_RGBA,	       GL_UNSIGNED_SHORT };
	f[PixelFormat::kRGB16Unsigned]	 = { GL_RGB16UI,	   GL_RGB,	       GL_UNSIGNED_SHORT };
	f[PixelFormat::kRG16Unsigned]	 = { GL_RG16UI,		   GL_RG,	       GL_UNSIGNED_SHORT };
	f[PixelFormat::kR16Unsigned]	 = { GL_R16UI,		   GL_RED,	       GL_UNSIGNED_SHORT };
	f[PixelFormat::kRGBA32Unsigned]	 = { GL_RGBA32UI,	   GL_RGBA,	       GL_UNSIGNED_INT };
	f[PixelFormat::kRGB32Unsigned]	 = { GL_RGB32UI,	   GL_RGB,	       GL_UNSIGNED_INT };
	f[PixelFormat::kRG32Unsigned]	 = { GL_RG32UI,		   GL_RG,	       GL_UNSIGNED_INT };
	f[PixelFormat::kR32Unsigned]	 = { GL_R32UI,		   GL_RED,	       GL_UNSIGNED_INT };
	f[PixelFormat::kDepth16]	 = { GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT };
	f[PixelFormat::kDepth24]	 = { GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT };
	f[PixelFormat::kDepth24Stencil8] = { GL_DEPTH24_STENCIL8,  GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8 };
}

#if ORION_GL_DEBUG

/** GL debug output callback.
 * @param source	Message source.
 * @param type		Message type.
 * @param id		Implementation-dependent ID.
 * @param severity	Severity of the message.
 * @param length	Length of the message.
 * @param message	Message text.
 * @param param		User-defined parameter (unused). */
GLEWAPIENTRY void GLGPUInterface::debug_callback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const GLvoid *param)
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
