/*
 * Copyright (C) 2015-2016 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               OpenGL GPU interface implementation.
 */

#include "gl.h"
#include "window.h"

#include "engine/engine.h"

/** Global GL GPU interface. */
GLGPUManager *g_opengl = nullptr;

/** Minimum and maximum supported OpenGL versions. */
static const int kGLMinMajorVersion = 3;
static const int kGLMinMinorVersion = 3;
static const int kGLMaxMajorVersion = 4;
static const int kGLMaxMinorVersion = 5;

/** Required OpenGL extensions. */
static const char *g_requiredGLExtensions[] = {
    "GL_ARB_separate_shader_objects",
    "GL_ARB_texture_storage",
    "GL_ARB_texture_view",
    "GL_EXT_texture_filter_anisotropic",
};

/** Create the GPU manager.
 * @param config        Engine configuration.
 * @param window        Where to store pointer to created window.
 * @return              Pointer to created GPU manager. */
GPUManager *GPUManager::create(const EngineConfiguration &config, Window *&window) {
    return new GLGPUManager(config, window);
}

/**
 * Identify the highest supported GL core profile version.
 *
 * Creates dummy windows/OpenGL contexts to identify the highest supported
 * OpenGL core profile versionm, and leaves the SDL_GL_* attributes set
 * accordingly.
 *
 * We want to create a core profile because OS X and Mesa only give 2.x support
 * when a compatibility profile is requested, unlike the NVIDIA driver which
 * gives the highest version it supports. However, if we request a core profile,
 * the NVIDIA driver gives the exact version requested. We want the highest
 * available version. Therefore, we must repeatedly try to recreate contexts
 * with different versions until we succeed.
 */
static void identifyGLCoreVersion() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    int majorVersion = kGLMaxMajorVersion;
    int minorVersion = kGLMaxMinorVersion;

    while (majorVersion > kGLMinMajorVersion ||
           (majorVersion == kGLMinMajorVersion && minorVersion >= kGLMinMinorVersion))
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majorVersion);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorVersion);

        SDL_Window *window = SDL_CreateWindow(nullptr,
                                              0, 0, 1, 1,
                                              SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN);
        if (!window)
            fatal("Failed to create dummy window: %s", SDL_GetError());

        SDL_GLContext context = SDL_GL_CreateContext(window);
        if (context)
            SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        if (context)
            return;

        if (minorVersion == 0) {
            majorVersion--;
            minorVersion = 3;
        } else {
            minorVersion--;
        }
    }

    fatal("OpenGL %u.%u or later is not supported", kGLMinMajorVersion, kGLMinMinorVersion);
}

/** Initialise the OpenGL GPU manager.
 * @param config        Engine configuration.
 * @param window        Where to store pointer to created window. */
GLGPUManager::GLGPUManager(const EngineConfiguration &config, Window *&window) :
    m_sdlContext        (nullptr),
    m_currentRenderPass (nullptr)
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

    /* Determine the OpenGL profile version to create. */
    identifyGLCoreVersion();

    #if ORION_GL_DEBUG
        /* If GL debugging is enabled, enable the debug context flag so
         * that we can use ARB_debug_output. */
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    #endif

    /* Create the window. */
    window = new GLWindow(config);

    m_sdlContext = SDL_GL_CreateContext(g_mainWindow->sdlWindow());
    if (!m_sdlContext)
        fatal("Failed to create GL context: %s", SDL_GetError());

    SDL_GL_SetSwapInterval(config.displayVsync);

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

            /* Enable all messages by default. */
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);

            /* Don't want to see push/pop group messages. */
            if (this->features[GLFeatures::kCapKHRDebug]) {
                glDebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_PUSH_GROUP, GL_DONT_CARE, 0, nullptr, false);
                glDebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_POP_GROUP, GL_DONT_CARE, 0, nullptr, false);
            }

            /* Only enable debug notifications if we want them. */
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                                  GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
                                  ORION_GL_DEBUG_NOTIFICATIONS);
        }
    #endif

    /* Create the default VAO. */
    glGenVertexArrays(1, &this->defaultVertexArray);
    this->state.bindVertexArray(this->defaultVertexArray);

    /* Set up some default state. FIXME? */
    this->state.enableCullFace(true);
    this->state.setCullFace(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    /* We want all outputs to sRGB textures to perform conversion. */
    glEnable(GL_FRAMEBUFFER_SRGB);
}

/** Shut down the GPU interface. */
GLGPUManager::~GLGPUManager() {
    if (m_sdlContext)
        SDL_GL_DeleteContext(m_sdlContext);

    g_opengl = nullptr;
}

/** Detect GL features and check requirements. */
void GLGPUManager::initFeatures() {
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

    /* Check for required extensions. */
    for (size_t i = 0; i < arraySize(g_requiredGLExtensions); i++) {
        if (!features[g_requiredGLExtensions[i]])
            fatal("Required OpenGL extension '%s' is not supported", g_requiredGLExtensions[i]);
    }

    /* Determine capabilities. */
    auto checkCapExtension =
        [&] (const std::string &extension, uint32_t cap) {
            if (features[extension])
                features.capabilities |= cap;
        };
    checkCapExtension("GL_KHR_debug", GLFeatures::kCapKHRDebug);

    /* Cache some GL information. */
    glGetIntegerv(GL_MAJOR_VERSION, &features.versionMajor);
    glGetIntegerv(GL_MINOR_VERSION, &features.versionMinor);
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &features.maxAnisotropy);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &features.maxTextureUnits);
}

/** Initialize the supported pixel format conversion table. */
void GLGPUManager::initPixelFormats() {
    auto &f = this->pixelFormats;

    /* TODO: For now this is a static table. We should identify the formats
     * that are actually supported, and have an engine generic table of
     * supported formats.
     * TODO: If we ever run on a big endian platform, the packed pixel
     * formats will need to be changed. */
    f[PixelFormat::kR8G8B8A8]          = { GL_RGBA8,              GL_RGBA,            GL_UNSIGNED_INT_8_8_8_8_REV };
    f[PixelFormat::kR8G8B8A8sRGB]      = { GL_SRGB8_ALPHA8,       GL_RGBA,            GL_UNSIGNED_INT_8_8_8_8_REV };
    f[PixelFormat::kR8G8]              = { GL_RG8,                GL_RG,              GL_UNSIGNED_BYTE };
    f[PixelFormat::kR8]                = { GL_R8,                 GL_RED,             GL_UNSIGNED_BYTE };
    f[PixelFormat::kB8G8R8A8]          = { GL_RGBA8,              GL_BGRA,            GL_UNSIGNED_INT_8_8_8_8_REV };
    f[PixelFormat::kB8G8R8A8sRGB]      = { GL_SRGB8_ALPHA8,       GL_BGRA,            GL_UNSIGNED_INT_8_8_8_8_REV };
    f[PixelFormat::kR10G10B10A2]       = { GL_RGB10_A2,           GL_RGBA,            GL_UNSIGNED_INT_2_10_10_10_REV };
    f[PixelFormat::kFloatR16G16B16A16] = { GL_RGBA16F,            GL_RGBA,            GL_HALF_FLOAT };
    f[PixelFormat::kFloatR16G16B16]    = { GL_RGB16F,             GL_RGB,             GL_HALF_FLOAT };
    f[PixelFormat::kFloatR16G16]       = { GL_RG16F,              GL_RG,              GL_HALF_FLOAT };
    f[PixelFormat::kFloatR16]          = { GL_R16F,               GL_RED,             GL_HALF_FLOAT };
    f[PixelFormat::kFloatR32G32B32A32] = { GL_RGBA32F,            GL_RGBA,            GL_FLOAT };
    f[PixelFormat::kFloatR32G32B32]    = { GL_RGB32F,             GL_RGB,             GL_FLOAT };
    f[PixelFormat::kFloatR32G32]       = { GL_RG32F,              GL_RG,              GL_FLOAT };
    f[PixelFormat::kFloatR32]          = { GL_R32F,               GL_RED,             GL_FLOAT };
    f[PixelFormat::kDepth16]           = { GL_DEPTH_COMPONENT16,  GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT };
    f[PixelFormat::kDepth32]           = { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT };
    f[PixelFormat::kDepth32Stencil8]   = { GL_DEPTH32F_STENCIL8,  GL_DEPTH_STENCIL,   GL_FLOAT_32_UNSIGNED_INT_24_8_REV };
}

#if ORION_GL_DEBUG

/** Filters on debug messages. */
static const char *kDebugMessageFilters[] = {
    /* This appears to be spurious on NVIDIA. Appears as "Texture 0 is..."
     * despite texture 0 not being bound nor used by the bound shader. */
    "is base level inconsistent. Check texture size.",
};

/** GL debug output callback.
 * @param source        Message source.
 * @param type          Message type.
 * @param id            Implementation-dependent ID.
 * @param severity      Severity of the message.
 * @param length        Length of the message.
 * @param message       Message text.
 * @param param         User-defined parameter (unused). */
void GLEWAPIENTRY GLGPUManager::debugCallback(GLenum source,
                                              GLenum type,
                                              GLuint id,
                                              GLenum severity,
                                              GLsizei length,
                                              const GLchar *message,
                                              const GLvoid *param)
{
    for (const char *filter : kDebugMessageFilters) {
        if (std::strstr(message, filter))
            return;
    }

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
