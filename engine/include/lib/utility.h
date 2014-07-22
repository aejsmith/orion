/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Utility functions/definitions.
 */

#ifndef ORION_LIB_UTILITY_H
#define ORION_LIB_UTILITY_H

#include "core/defs.h"

#include <cstdarg>
#include <string>

namespace util {

/** Get the size of an array.
 * @param array		Array to get size of. */
template <typename T, size_t N>
constexpr size_t array_size(T (&array)[N]) {
	return N;
}

/** Round a value up.
 * @param val		Value to round.
 * @param nearest	Boundary to round up to.
 * @return		Rounded value. */
template <typename T, typename U>
inline T round_up(T val, const U &nearest) {
	/* When nearest is a power of 2, this is optimised to be equivalent
	 * to the following:
	 *  if(val & (nearest - 1)) {
	 *  	val += nearest;
	 *  	val &= ~(nearest - 1);
	 *  }
	 * Using the implementation below has the benefit that we do not limit
	 * use to power-of-2 alignment. */
	if(val % (nearest)) {
		val -= val % (nearest);
		val += nearest;
	}

	return val;
}

/** Round a value down.
 * @param val		Value to round.
 * @param nearest	Boundary to round down to.
 * @return		Rounded value. */
template <typename T, typename U>
inline T round_down(T val, const U &nearest) {
	/* Same as above. */
	if(val % (nearest))
		val -= val % (nearest);

	return val;
}

/** Check if a value is a power of 2.
 * @param val		Value to check.
 * @return		Whether value is a power of 2. */
template <typename T>
inline bool is_pow2(T val) {
	return ((val) && ((val) & ((val) - 1)) == 0);
}

extern std::string format(const char *fmt, va_list args);
extern std::string format(const char *fmt, ...);

}

#endif /* ORION_LIB_UTILITY_H */
