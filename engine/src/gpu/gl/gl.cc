/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               OpenGL GPU interface implementation.
 */

#include "gl.h"

#include "engine/window.h"

/** Global GL GPU interface. */
GLGPUInterface *g_opengl = nullptr;

/** Target GL major version. */
static const int kGLMajorVersion = 3;

/** Target GL minor version. */
static const int kGLMinorVersion = 3;

/** Required OpenGL extensions. */
static const char *g_requiredGLExtensions[] = {
    "GL_ARB_separate_shader_objects",
    "GL_ARB_texture_storage",
    "GL_EXT_texture_filter_anisotropic",
};

/** Initialize the GPU interface. */
GLGPUInterface::GLGPUInterface() :
    defaultVertexArray(0),
    m_sdlContext(nullptr)
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
    if (m_sdlContext)
        SDL_GL_DeleteContext(m_sdlContext);

    g_opengl = nullptr;
}

/** Initialize the GPU interface. */
void GLGPUInterface::init() {
    m_sdlContext = SDL_GL_CreateContext(g_mainWindow->sdlWindow());
    if (!m_sdlContext)
        fatal("Failed to create GL context: %s", SDL_GetError());

    SDL_GL_SetSwapInterval(0);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        fatal("Failed to initialize GLEW");

    /* Initialize the features table and check requirements. */
    initFeatures();
    this->state.initResources(this->features);

    /* Populate the pixel format table. */
    initPixelFormats();

    #if ORION_GL_DEBUG
        /* Hook up debug output if supported. */
        if (this->features["GL_ARB_debug_output"]) {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback((GLDEBUGPROC)debugCallback, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
                ORION_GL_DEBUG_NOTIFICATIONS);
        }
    #endif

    /* Create the default VAO. */
    glGenVertexArrays(1, &this->defaultVertexArray);
    this->state.bindVertexArray(this->defaultVertexArray);

    /* Set up initial render target/viewport state (main window). */
    setRenderTarget(nullptr);

    /* Set up some default state. FIXME */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

/** Detect GL features and check requirements. */
void GLGPUInterface::initFeatures() {
    GLFeatures &features = this->features;

    /* Log some OpenGL details. */
    logInfo("OpenGL vendor:   %s", glGetString(GL_VENDOR));
    logInfo("OpenGL renderer: %s", glGetString(GL_RENDERER));
    logInfo("OpenGL version:  %s", glGetString(GL_VERSION));

    /* Query supported extensions. */
    GLint count = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &count);
    for (GLint i = 0; i < count; i++) {
        std::string extension((const char *)glGetStringi(GL_EXTENSIONS, i));
        features.extensions.insert(extension);
    }

    /* Print out a (sorted) list of the extensions found. */
    logDebug("OpenGL extensions:");
    for (const std::string &extension : features.extensions)
        logDebug("  %s", extension.c_str());

    /* Check whether the version number is high enough. */
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < kGLMajorVersion || (major == kGLMajorVersion && minor < kGLMinorVersion))
        fatal("OpenGL version %d.%d is required", kGLMajorVersion, kGLMinorVersion);

    /* Check for required extensions. */
    for (size_t i = 0; i < util::arraySize(g_requiredGLExtensions); i++) {
        if (!this->features[g_requiredGLExtensions[i]])
            fatal("Required OpenGL extension '%s' is not supported", g_requiredGLExtensions[i]);
    }

    /* Cache some GL information. */
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &features.maxAnisotropy);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &features.maxTextureUnits);
}

/** Initialize the supported pixel format conversion table. */
void GLGPUInterface::initPixelFormats() {
    PixelFormatArray &f = this->pixelFormats;

    /* TODO: For now this is a static table. We should identify the formats
     * that are actually supported, and have an engine generic table of
     * supported formats.
     * TODO: If we ever run on a big endian platform, the packed pixel
     * formats will need to be changed. */
    f[PixelFormat::kR8G8B8A8]          = { GL_RGBA8,             GL_RGBA,            GL_UNSIGNED_INT_8_8_8_8_REV };
    f[PixelFormat::kR8G8B8]            = { GL_RGB8,              GL_RGB,             GL_UNSIGNED_BYTE };
    f[PixelFormat::kR8G8]              = { GL_RG8,               GL_RG,              GL_UNSIGNED_BYTE };
    f[PixelFormat::kR8]                = { GL_R8,                GL_RED,             GL_UNSIGNED_BYTE };
    f[PixelFormat::kB8G8R8A8]          = { GL_RGBA8,             GL_BGRA,            GL_UNSIGNED_INT_8_8_8_8_REV };
    f[PixelFormat::kB8G8R8]            = { GL_RGB8,              GL_BGR,             GL_UNSIGNED_BYTE };
    f[PixelFormat::kFloatR16G16B16A16] = { GL_RGBA16F,           GL_RGBA,            GL_HALF_FLOAT };
    f[PixelFormat::kFloatR16G16B16]    = { GL_RGB16F,            GL_RGB,             GL_HALF_FLOAT };
    f[PixelFormat::kFloatR16G16]       = { GL_RG16F,             GL_RG,              GL_HALF_FLOAT };
    f[PixelFormat::kFloatR16]          = { GL_R16F,              GL_RED,             GL_HALF_FLOAT };
    f[PixelFormat::kFloatR32G32B32A32] = { GL_RGBA32F,           GL_RGBA,            GL_FLOAT };
    f[PixelFormat::kFloatR32G32B32]    = { GL_RGB32F,            GL_RGB,             GL_FLOAT };
    f[PixelFormat::kFloatR32G32]       = { GL_RG32F,             GL_RG,              GL_FLOAT };
    f[PixelFormat::kFloatR32]          = { GL_R32F,              GL_RED,             GL_FLOAT };
    f[PixelFormat::kDepth16]           = { GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT };
    f[PixelFormat::kDepth24]           = { GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT };
    f[PixelFormat::kDepth24Stencil8]   = { GL_DEPTH24_STENCIL8,  GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8 };
}

#if ORION_GL_DEBUG

/** GL debug output callback.
 * @param source        Message source.
 * @param type          Message type.
 * @param id            Implementation-dependent ID.
 * @param severity      Severity of the message.
 * @param length        Length of the message.
 * @param message       Message text.
 * @param param         User-defined parameter (unused). */
GLEWAPIENTRY void GLGPUInterface::debugCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const GLvoid *param)
{
    const char *sourceString = "OTHER";
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            sourceString = "API";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceString = "SHADER_COMPILER";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceString = "WINDOW_SYSTEM";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceString = "THIRD_PARTY";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceString = "APPLICATION";
            break;
    }

    const char *typeString = "OTHER";
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            typeString = "ERROR";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            typeString = "DEPRECATED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            typeString = "UNDEFINED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            typeString = "PERFORMANCE";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            typeString = "PORTABILITY";
            break;
    }

    LogLevel level = LogLevel::kDebug;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            level = LogLevel::kError;
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
        case GL_DEBUG_SEVERITY_LOW:
            level = LogLevel::kWarning;
            break;
    }

    logWrite(level, "GL [source = %s, type = %s]:", sourceString, typeString);
    logWrite(level, "%s", message);

    if (severity == GL_DEBUG_SEVERITY_HIGH)
        fatal("GL driver error (see log for details)");
}

#endif /* ORION_GL_DEBUG */
