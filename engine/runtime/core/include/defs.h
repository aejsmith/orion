/*
 * Copyright (C) 2015-2017 Alex Smith
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
 * @brief               Core engine definitions.
 *
 * This file pulls in commonly used system headers to avoid having to include
 * them everywhere. It also has a few definitions that are used everywhere. If
 * a header has no other includes, it should at least include this header.
 */

#pragma once

#include <glm.h>

#include <cinttypes>
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

#if defined(__GNUC__)
    #define NORETURN                __attribute__((noreturn))
    #define FORCEINLINE             __attribute__((always_inline))
    #define NOINLINE                __attribute__((noinline))
    #define likely(x)               __builtin_expect(!!(x), 1)
    #define unlikely(x)             __builtin_expect(!!(x), 0)
    #define compiler_unreachable()  __builtin_unreachable()
#elif defined(_MSC_VER)
    #define NORETURN                __declspec(noreturn)
    #define FORCEINLINE             __forceinline
    #define NOINLINE                __declspec(noinline)
    #define likely(x)               (x)
    #define unlikely(x)             (x)
    #define compiler_unreachable()  abort()
#else
    #error "Compiler is not supported"
#endif

/** Target bitness definition. */
#if INTPTR_MAX == INT32_MAX
    #define ORION_32BIT 1
#elif INTPTR_MAX == INT64_MAX
    #define ORION_64BIT 1
#else
    #error "Cannot determine target bitness"
#endif

/** Target endianness definition. */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define ORION_LITTLE_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define ORION_BIG_ENDIAN 1
#else
    #error "Cannot determine target endianness"
#endif

/**
 * Error handling definitions.
 */

NORETURN extern void __fatal(const char *file, int line, const char *fmt, ...);

/**
 * Signal that an unrecoverable error has occurred.
 *
 * This function should be called to indicate that an unrecoverable error has
 * occurred at runtime. It results in an immediate shut down of the engine and
 * displays an error message to the user in release builds, and calls abort()
 * on debug builds to allow the error to be caught in a debugger. This function
 * does not return.
 *
 * @param fmt           Error message format string.
 * @param ...           Arguments to substitute into format.
 */
#define fatal(fmt, ...) \
    __fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#ifdef ORION_BUILD_DEBUG

/**
 * Check that a condition is true.
 *
 * Check that a condition is true. If it is not, the engine will abort with an
 * error message giving the condition that failed.
 *
 * @param cond          Condition to check.
 */
#define check(cond) \
    do { \
        if (unlikely(!(cond))) \
            __fatal(__FILE__, __LINE__, "Assertion failed: %s", #cond); \
    } while (0)

/**
 * Check that a condition is true.
 *
 * Check that a condition is true. If it is not, the engine will abort with the
 * specified message.
 *
 * @param cond          Condition to check.
 * @param fmt           Error message format string.
 * @param ...           Arguments to substitute into format.
 */
#define checkMsg(cond, fmt, ...) \
    do { \
        if (unlikely(!(cond))) \
            __fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)

/**
 * Mark that a point is unreachable, raise an error if not.
 *
 * This hints to the compiler that the point where it is used is unreachable,
 * if it is reached then a fatal error will be raised in debug, otherwise in
 * release it will be undefined behaviour.
 */
#define unreachable()       fatal("unreachable() statement hit");

/**
 * Mark that a point is unreachable, raise an error if not.
 *
 * This hints to the compiler that the point where it is used is unreachable,
 * if it is reached then a fatal error will be raised in debug, otherwise in
 * release it will be undefined behaviour.
 */
#define unreachableMsg(...) fatal(__VA_ARGS__);

#else /* ORION_BUILD_DEBUG */

#define check(cond)              do { (void)sizeof(cond); } while(0)
#define checkMsg(cond, fmt, ...) check(cond)
#define unreachable()            compiler_unreachable()
#define unreachableMsg(...)      compiler_unreachable()

#endif /* ORION_BUILD_DEBUG */

/**
 * Helper macros.
 */

/** Mark that a variable may be unused (e.g. in release builds). */
#define unused(var) ((void)(var))

/**
 * Simple utility classes.
 */

/** Base class ensuring derived classes are not copyable. */
struct Noncopyable {
    Noncopyable &operator=(const Noncopyable &) = delete;
    Noncopyable(const Noncopyable &) = delete;
    Noncopyable() = default;
};
