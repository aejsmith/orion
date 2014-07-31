/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Logging functions.
 */

#ifndef ORION_LIB_LOG_H
#define ORION_LIB_LOG_H

#include "lib/noncopyable.h"

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

/** Write a log message.
 * @param level		Message log level.
 * @param fmt		Message format string.
 * @param ...		Arguments to substitute into format string. */
#define orion_log(level, fmt, ...) \
	g_engine->log()->write(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif /* ORION_LIB_LOG_H */
