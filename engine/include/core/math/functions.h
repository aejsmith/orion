/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Math utility functions.
 */

#pragma once

#include "core/defs.h"

namespace math {

/** Round a value up.
 * @param val		Value to round.
 * @param nearest	Boundary to round up to.
 * @return		Rounded value. */
template <typename T, typename U>
inline constexpr T roundUp(const T &val, const U &nearest) {
	/* When nearest is a power of 2, this is optimised to be equivalent
	 * to the following:
	 *  if(val & (nearest - 1)) {
	 *  	val += nearest;
	 *  	val &= ~(nearest - 1);
	 *  }
	 * Using the implementation below has the benefit that we do not limit
	 * use to power-of-2 alignment. */
	return (val % nearest)
		? (val - (val % nearest)) + nearest
		: val;
}

/** Round a value down.
 * @param val		Value to round.
 * @param nearest	Boundary to round down to.
 * @return		Rounded value. */
template <typename T, typename U>
inline constexpr T roundDown(const T &val, const U &nearest) {
	/* Same as above. */
	return (val % nearest)
		? val - (val % nearest)
		: val;
}

/** Check if a value is a power of 2.
 * @param val		Value to check.
 * @return		Whether value is a power of 2. */
template <typename T>
inline constexpr bool isPow2(const T &val) {
	return val && (val & (val - 1)) == 0;
}

}
