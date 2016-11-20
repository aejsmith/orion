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
 * @brief               Logging functions.
 */

#pragma once

#include "core/defs.h"

/** Log level definitions. */
enum class LogLevel {
    kDebug,
    kInfo,
    kWarning,
    kError,
};

/** Class implementing logging. */
class LogManager : Noncopyable {
public:
    LogManager();
    ~LogManager();

    void write(LogLevel level, const char *file, int line, const char *fmt, ...);
};

extern LogManager *g_logManager;

/** Write a debug log message.
 * @param fmt           Message format string.
 * @param ...           Arguments to substitute into format string. */
#define logDebug(fmt, ...) \
    g_logManager->write(LogLevel::kDebug, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/** Write an informational log message.
 * @param fmt           Message format string.
 * @param ...           Arguments to substitute into format string. */
#define logInfo(fmt, ...) \
    g_logManager->write(LogLevel::kInfo, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/** Write a warning log message.
 * @param fmt           Message format string.
 * @param ...           Arguments to substitute into format string. */
#define logWarning(fmt, ...) \
    g_logManager->write(LogLevel::kWarning, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/** Write an error log message.
 * @param fmt           Message format string.
 * @param ...           Arguments to substitute into format string. */
#define logError(fmt, ...) \
    g_logManager->write(LogLevel::kError, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/** Write a log message with a specified level.
 * @param level         Log level to write with.
 * @param fmt           Message format string.
 * @param ...           Arguments to substitute into format string. */
#define logWrite(level, fmt, ...) \
    g_logManager->write(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
