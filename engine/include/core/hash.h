/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Hash functions.
 */

#pragma once

#include "core/core.h"

extern size_t hashMem(const void *buf, size_t len);

/** Hash a simple integer value.
 * @param value		Value to hash.
 * @return		Generated hash. */
template <typename T>
inline typename std::enable_if<std::is_integral<T>::value, size_t>::type hashValue(T value) {
	/* This is probably reasonable for many cases. */
	return static_cast<size_t>(value);
}

/** Hash an enum value.
 * @param value		Value to hash.
 * @return		Generated hash. */
template <typename T>
inline typename std::enable_if<std::is_enum<T>::value, size_t>::type hashValue(T value) {
	return hashValue(static_cast<typename std::underlying_type<T>::type>(value));
}

/** Hash a floating point value.
 * @param value		Value to hash.
 * @return		Generated hash. */
template <typename T>
inline typename std::enable_if<std::is_floating_point<T>::value, size_t>::type hashValue(T value) {
	/* Generate the same hash for -0.0 and 0.0. */
	if(value == 0)
		return 0;
	return hashMem(&value, sizeof(value));
}

/** Hash a pointer.
 * @note		This will hash the pointer itself, not whatever it
 *			points to.
 * @param value		Value to hash.
 * @return		Generated hash. */
template <typename T>
inline size_t hashValue(T *value) {
	return hashMem(&value, sizeof(value));
}

/** Hash a string.
 * @param value		Value to hash.
 * @return		Generated hash. */
inline size_t hashValue(const std::string &value) {
	return hashMem(value.c_str(), value.length());
}

/**
 * Combine hash values.
 *
 * This function can be called repeatedly to generate a hash value from several
 * variables, like so:
 *
 *  size_t hash = hashValue(valA);
 *  hash = hashCombine(hash, valB);
 *  hash = hashCombine(hash, valC);
 *  ...
 *
 * It uses hashValue() to hash the value. Note that for values that are
 * contiguous in memory it is likely faster to hash the entire range using
 * hashMem().
 *
 * @param seed		Existing hash value.
 * @param value		New value to hash.
 *
 * @return		Generated hash.
 */
template <typename T>
inline size_t hashCombine(size_t seed, const T &value) {
	size_t hash = hashValue(value);

	#if ORION_64BIT
		/* Equivalent to CityHash64WithSeed. */
		const size_t c1 = 0x9ae16a3b2f90404full;
		const size_t c2 = 0x9ddfea08eb382d69ull;

		size_t a = ((hash - c1) ^ seed) * c2;
		a ^= (a >> 47);
		size_t b = (seed ^ a) * c2;
		b ^= (b >> 47);
		b *= c2;
		return b;
	#else
		/* MurmurHash3. */
		const size_t c1 = 0xcc9e2d51;
		const size_t c2 = 0x1b873593;
		const size_t c3 = 0xe6546b64;

		hash *= c1;
		hash = (hash << 15) | (hash >> (32 - 15));
		hash *= c2;

		seed ^= k1;
		seed = (seed << 13) | (seed >> (32 - 13));
		seed = seed * 5 + c3;
		return seed;
	#endif
}

/**
 * Hash functor for use with containers.
 *
 * Provides a hash functor for use with hash-based containers which require a
 * function object for calculating a hash. Works with any type for which
 * hashValue() is defined.
 *
 * @tparam T		Type of the value to hash.
 */
template <typename T>
struct Hash {
	typedef T argument_type;
	typedef size_t result_type;

	/** Calculate the hash.
	 * @param value		Value to calculate hash of.
	 * @return		Calculated hash. */
	result_type operator()(const argument_type &value) const {
		return hashValue(value);
	}
};
