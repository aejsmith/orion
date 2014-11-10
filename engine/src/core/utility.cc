/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Utility functions.
 */

#include "core/utility.h"

#include <cstdio>

namespace util {

/** Format a string and return it in a std::string.
 * @param fmt           Format string.
 * @param args          Values to substitute into string.
 * @return              Formatted string. */
std::string format(const char *fmt, va_list args) {
    char buf[8192];
    vsnprintf(buf, 8192, fmt, args);
    return std::string(buf);
}

/** Format a string and return it in a std::string.
 * @param fmt           Format string.
 * @param ...           Values to substitute into string.
 * @return              Formatted string. */
std::string format(const char *fmt, ...) {
    char buf[8192];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, 8192, fmt, args);
    va_end(args);

    return std::string(buf);
}

}
