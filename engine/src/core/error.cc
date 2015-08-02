/*
 * Copyright (C) 2015 Alex Smith
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
 * @brief               Error handling functions.
 */

#include "core/core.h"
#include "core/string.h"

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
 * @param fmt           Format string for error message.
 * @param ...           Arguments to substitute into format string.
 */
void __fatal(const char *file, int line, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    std::string str = String::format("Fatal Error (at %s:%d): ", file, line) + String::format(fmt, args);
    va_end(args);

    if (g_logManager) {
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
