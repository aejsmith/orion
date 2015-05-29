/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Logging functions.
 *
 * @todo                This is pretty simple for now. In future it will support
 *                      multiple log outputs (terminal, log file, in-engine
 *                      console) by registering log listeners which would
 *                      receive log messages.
 */

#include "core/log.h"
#include "core/string.h"

#include <ctime>

/** Global log manager instance. */
EngineGlobal<LogManager> g_logManager;

/** Initialize the log manager. */
LogManager::LogManager() {}

/** Destroy the log manager. */
LogManager::~LogManager() {}

/** Write a message to the log.
 * @param level         Log level.
 * @param file          File in which the message was written.
 * @param line          Line at which the message was written.
 * @param fmt           Message format string.
 * @param ...           Arguments to substitute into format string. */
void LogManager::write(LogLevel level, const char *file, int line, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    std::string msg = String::format(fmt, args);
    va_end(args);

    time_t t = time(nullptr);
    struct tm local;
    localtime_r(&t, &local);

    char timeString[20];
    strftime(timeString, 20, "%Y-%m-%d %H:%M:%S", &local);

    const char *levelString = "";
    switch (level) {
        case LogLevel::kDebug:
            levelString = "\033[1;30m";
            break;
        case LogLevel::kInfo:
            levelString = "\033[1;34m";
            break;
        case LogLevel::kWarning:
            levelString = "\033[1;33m";
            break;
        case LogLevel::kError:
            levelString = "\033[1;31m";
            break;
    }

    fprintf((level < LogLevel::kError) ? stdout : stderr,
        "%s%s \033[0m%s\n",
        levelString, timeString, msg.c_str());
}
