/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Utility functions/definitions.
 */

#pragma once

#include "core/defs.h"

namespace util {

/** Get the size of an array.
 * @param array		Array to get size of. */
template <typename T, size_t N>
constexpr size_t arraySize(T (&array)[N]) {
	return N;
}

extern std::string format(const char *fmt, va_list args);
extern std::string format(const char *fmt, ...);

}
