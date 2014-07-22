/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Error handling functions/definitions.
 */

#ifndef ORION_LIB_ERROR_H
#define ORION_LIB_ERROR_H

#include "lib/compiler.h"

extern void __orion_abort(const char *file, int line, const char *fmt, ...) NORETURN;

/**
 * Signal that an unrecoverable error has occurred.
 *
 * This function should be called to indicate that an unrecoverable error has
 * occurred at runtime. It results in an immediate shut down of the engine and
 * displays an error message to the user in release builds, and calls abort()
 * on debug builds to allow the error to be caught in a debugger. This function
 * does not return.
 *
 * @param fmt		Error message format string.
 * @param ...		Arguments to substitute into format.
 */
#define orion_abort(fmt, ...) \
	__orion_abort(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * Check that a condition is true.
 *
 * Check that a condition is true. If it is not, the engine will abort with an
 * error message giving the condition that failed.
 *
 * @param cond		Condition to check.
 */
#define orion_assert(cond) \
	do { \
		if(unlikely(!(cond))) \
			__orion_abort(__FILE__, __LINE__, "Assertion failed: %s", #cond); \
	} while(0)

/**
 * Check that a condition is true.
 *
 * Check that a condition is true. If it is not, the engine will abort with the
 * specified message.
 *
 * @param cond		Condition to check.
 * @param fmt		Error message format string.
 * @param ...		Arguments to substitute into format.
 */
#define orion_check(cond, fmt, ...) \
	do { \
		if(unlikely(!(cond))) \
			__orion_abort(__FILE__, __LINE__, fmt, ##__VA_ARGS__); \
	} while(0)

#endif /* ORION_LIB_ERROR_H */
