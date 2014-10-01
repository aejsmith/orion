/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Logging functions.
 */

#pragma once

#include "core/engine_global.h"

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

extern EngineGlobal<LogManager> g_logManager;

/** Write a debug log message.
 * @param fmt		Message format string.
 * @param ...		Arguments to substitute into format string. */
#define logDebug(fmt, ...) \
	g_logManager->write(LogLevel::kDebug, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/** Write an informational log message.
 * @param fmt		Message format string.
 * @param ...		Arguments to substitute into format string. */
#define logInfo(fmt, ...) \
	g_logManager->write(LogLevel::kInfo, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/** Write a warning log message.
 * @param fmt		Message format string.
 * @param ...		Arguments to substitute into format string. */
#define logWarning(fmt, ...) \
	g_logManager->write(LogLevel::kWarning, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/** Write an error log message.
 * @param fmt		Message format string.
 * @param ...		Arguments to substitute into format string. */
#define logError(fmt, ...) \
	g_logManager->write(LogLevel::kError, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/** Write a log message with a specified level.
 * @param level		Log level to write with.
 * @param fmt		Message format string.
 * @param ...		Arguments to substitute into format string. */
#define logWrite(level, fmt, ...) \
	g_logManager->write(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
