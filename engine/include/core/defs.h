/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Core engine definitions.
 *
 * This file pulls in commonly used system headers to avoid having to include
 * them everywhere. It also has a few definitions that are used everywhere. If
 * a header has no other includes, it should at least include this header.
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <string>

/**
 * Compiler definitions.
 */

#ifdef __GNUC__
# define PACKED			__attribute__((packed))
# define ALIGNED(a)		__attribute__((aligned(a)))
# define NORETURN		__attribute__((noreturn))
# define FORCEINLINE		__attribute__((always_inline))
# define likely(x)		__builtin_expect(!!(x), 1)
# define unlikely(x)		__builtin_expect(!!(x), 0)
# define unreachable()		__builtin_unreachable()
#else
# error "Compiler is not supported"
#endif

/** Target bitness definition. */
#if INTPTR_MAX == INT32_MAX
# define ORION_32BIT		1
#elif INTPTR_MAX == INT64_MAX
# define ORION_64BIT		1
#else
# error "Cannot determine target bitness"
#endif

/** Target endianness definition. */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define ORION_LITTLE_ENDIAN	1
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
# define ORION_BIG_ENDIAN	1
#else
# error "Cannot determine target endianness"
#endif

/**
 * Engine version definitions.
 */

extern const char *g_versionString;
extern const char *g_versionTimestamp;

/**
 * Error handling definitions.
 */

extern void __fatal(const char *file, int line, const char *fmt, ...) NORETURN;

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
#define fatal(fmt, ...) \
	__fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * Check that a condition is true.
 *
 * Check that a condition is true. If it is not, the engine will abort with an
 * error message giving the condition that failed.
 *
 * @param cond		Condition to check.
 */
#define check(cond) \
	do { \
		if(unlikely(!(cond))) \
			__fatal(__FILE__, __LINE__, "Assertion failed: %s", #cond); \
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
#define checkMsg(cond, fmt, ...) \
	do { \
		if(unlikely(!(cond))) \
			__fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__); \
	} while(0)

/**
 * Simple utility classes.
 */

/** Base class ensuring derived classes are not copyable. */
struct Noncopyable {
	Noncopyable &operator=(const Noncopyable &) = delete;
	Noncopyable(const Noncopyable &) = delete;
	Noncopyable() = default;
};
