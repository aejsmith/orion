/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Error handling functions.
 */

#include "core/core.h"

#include <SDL.h>

#include <cstdlib>

/**
 * Signal that an unrecoverable error has occurred.
 *
 * This function should be called to indicate that an unrecoverable error has
 * occurred at runtime. It results in an immediate shut down of the engine and
 * displays an error message to the user. This function does not return.
 *
 * @param
 * @param fmt		Format string for error message.
 * @param ...		Arguments to substitute into format string.
 */
void __fatal(const char *file, int line, const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	std::string str = util::format("Fatal Error (at %s:%d): ", file, line) + util::format(fmt, args);
	va_end(args);

	if(g_logManager) {
		g_logManager->write(LogLevel::kError, file, line, "%s", str.c_str());
	} else {
		fprintf(stderr, "%s\n", str.c_str());
	}

	#ifdef ORION_BUILD_DEBUG
		/* For a debug build, we can core dump or break into the
		 * debugger. */
		abort();
	#else
		/* This works even when SDL is not initialized. */
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", str.c_str(), NULL);
		_Exit(EXIT_FAILURE);
	#endif
}
